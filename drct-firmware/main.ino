#include "Adafruit_GPS.h"
#include "Adafruit_LIS3DH.h"
#include "GPS_Math.h"

#include "Grove-Ultrasonic-Ranger.h"

#include "google-maps-device-locator.h"

#include <math.h>
#include "math.h"
#include <ctype.h>

// CONFIGURATION
#define RANGEFINDER_ENABLED true
#define PUBLISH_DELAY (1 * 60) // IN SECONDS
#define CELL_LOCATION_QUERY_DELAY (1 * 60) // IN SECONDS
#define NO_MOTION_IDLE_SLEEP_DELAY (3 * 60) // IN SECONDS
#define HOW_LONG_SHOULD_WE_SLEEP (12 * 60 * 60) // IN SECONDS
#define CLICKTHRESHHOLD 50

PRODUCT_ID(9903);
PRODUCT_VERSION(4);
String deviceName = "";

// #define STARTING_LATITUDE_LONGITUDE_ALTITUDE NULL
uint8_t internalANT[]={0xB5,0x62,0x06,0x13,0x04,0x00,0x00,0x00,0xF0,0x7D,0x8A,0x2A};
uint8_t externalANT[]={0xB5,0x62,0x06,0x13,0x04,0x00,0x01,0x00,0xF0,0x7D,0x8B,0x2E};

#define mySerial Serial1
Adafruit_GPS GPS(&mySerial);
Adafruit_LIS3DH accel = Adafruit_LIS3DH(A2, A5, A4, A3);
FuelGauge fuel;
GoogleMapsDeviceLocator locator;
bool usingCellularLocation = false;
Ultrasonic ultrasonic(1); //Grove Port 1
time_t lastMotion = 0;
time_t lastPublish = 0;
time_t now = Time.now();
float cellularLatitude;
float cellularLongitude;
float cellularAccuracy;
CellularSignal signalInfo;
bool batteryAlert;


// lets keep the radio off until we get a fix, or 2 minutes go by.
SYSTEM_MODE(SEMI_AUTOMATIC);
// STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));

void setup() {
    // mirror RGB PINS
    RGB.mirrorTo(B3,B2,B1,true, true);

    // POWER TEMPERATURE SENSOR
    pinMode(A1,OUTPUT);
    pinMode(A0,INPUT);
    pinMode(B5,OUTPUT);
    digitalWrite(B5, HIGH);
    digitalWrite(A1, LOW);

    // electron asset tracker shield needs this to enable the power to the gps module.
    pinMode(D6, OUTPUT);
    digitalWrite(D6, LOW);

    // for blinking.
    pinMode(D7, OUTPUT);
    digitalWrite(D7, LOW);

    // wait a little for the GPS to wakeup
    delay(250);

    GPS.begin(9600);
    mySerial.begin(9600);
    Serial.begin(9600);

    //# request a HOT RESTART, in case we were in standby mode before.
    GPS.sendCommand("$PMTK101*32");
    delay(250);

    // request everything!
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_ALLDATA);
    delay(250);

    // turn off antenna updates
    GPS.sendCommand(PGCMD_NOANTENNA);
    delay(250);

    // select internal antenna
    //antennaSelect(internalANT);
    //antennaSelect(externalANT);

    initAccel();
    // locator.withSubscribe(locationCallback).withLocateOnce;
    locator.withSubscribe(locationCallback).withLocatePeriodic(CELL_LOCATION_QUERY_DELAY);

    Particle.function("status", sensorStatus);
}


void loop() {
    now = Time.now();
    signalInfo = Cellular.RSSI();
    batteryAlert = (fuel.getSoC()<10);

    checkGPS();

    // On boot / wake we publish
    if (lastPublish == 0) {
      if (Particle.connected() == false) {
          Particle.connect();
      }
      delay(10*1000);
      Particle.publish("status", "wake", 60, PRIVATE);
      lastPublish = now;
      lastMotion = now; // start sleep timer
      Particle.subscribe("particle/device/name", deviceNameHandler);
      Particle.publish("particle/device/name");
    }

// motion trigger wakeup
    bool hasMotion = digitalRead(WKP);
    digitalWrite(D7, (hasMotion) ? HIGH : LOW);
    if (hasMotion) {
        lastMotion = now;
    }

    // have we published recently?
    //Serial.println("lastPublish is " + String(lastPublish));
    if (now - lastPublish > PUBLISH_DELAY) {
        lastPublish = now;
        publishData("publish");
    }

// It's time to sleep
    if (now - lastMotion > NO_MOTION_IDLE_SLEEP_DELAY) {
        publishData("sleep");
        // Hey GPS, please stop using power, kthx.
        digitalWrite(D6, HIGH);
        // lets give ourselves a chance to settle, deal with anything pending, achieve enlightenment...
        delay(10*1000);
        System.sleep(SLEEP_MODE_DEEP, HOW_LONG_SHOULD_WE_SLEEP);
    }

    delay(10);

    locator.loop(); // WE need to refactor this to only call when we don't have a GPS FIX, significantly reducing calls to service and charges
}


void checkGPS() {
    while (mySerial.available()) {
        char c = GPS.read();
        if (GPS.newNMEAreceived()) {
            GPS.parse(GPS.lastNMEA());
            if (GPS.latitude != 0) {
                usingCellularLocation = false;
            }
        }
    }
}

void initAccel() {
    accel.begin(LIS3DH_DEFAULT_ADDRESS);

    // Default to 5kHz low-power sampling
    accel.setDataRate(LIS3DH_DATARATE_LOWPOWER_5KHZ);

    // Default to 4 gravities range
    accel.setRange(LIS3DH_RANGE_4_G);

    // listen for single-tap events at the threshold
    // keep the pin high for 1s, wait 1s between clicks

    //uint8_t c, uint8_t clickthresh, uint8_t timelimit, uint8_t timelatency, uint8_t timewindow
    accel.setClick(1, CLICKTHRESHHOLD);//, 0, 100, 50);
}

void publishData(String type) {

    bool motionTriggered = (now - lastMotion < PUBLISH_DELAY);
    long RangeInInches = 0;
    // RangeInInches = ultrasonic.MeasureInInches();
    float latitude, longitude, accuracy;

    if (usingCellularLocation) {
        latitude = cellularLatitude;
        longitude = cellularLongitude;
        accuracy = cellularAccuracy;
    }
    else {
        latitude = convertDegMinToDecDeg(GPS.latitude);
        longitude = convertDegMinToDecDeg(GPS.longitude);
        accuracy = GPS.fixquality;
    }

    accel.read();
    float aX = accel.x;
    float aY = accel.y;
    float aZ = accel.z;

    float temperatureC = ((3300*analogRead(A0)/4096.0)-500)/10.0;
    float temperatureF = (temperatureC * (9/5)) + 32;

    if ((latitude != 0) && (longitude != 0)) {
      String sensor_payload =
            "{\"type\":"                    + String("\"" + type + "\"")
          + ",\"deviceId\":"                + String("\"" + deviceName + "\"")
          + ",\"timestamp\":"               + String("\"" + Time.format(now, TIME_FORMAT_ISO8601_FULL) + "\"")
          + ",\"latitude\":"                + String(latitude)
          + ",\"longitude\":"               + String(longitude)
          + ",\"accuracy\":"                + String(accuracy)
          + ",\"altitude\":"                + String(GPS.altitude)
          + ",\"fixQuality\":"              + String(GPS.fixquality)
          + ",\"velocity\":"                + String(GPS.speed)
          + ",\"usingCellularLocation\":"   + String(usingCellularLocation)
          + ",\"motionTriggered\":"         + String(motionTriggered)
          + ",\"satellites\": "             + String(GPS.satellites)
          + ",\"batteryVoltage\":"          + String(fuel.getVCell())
          + ",\"batteryPercentCharge\":"    + String(fuel.getSoC())
          + ",\"batteryAlert\":"            + String(batteryAlert)
          + ",\"cellSignalStrength\":"      + String(signalInfo.rssi)
          + ",\"cellQuality\":"             + String(signalInfo.qual)
          + ",\"temperatureF\":"            + String::format("%f", temperatureF)
          + ",\"x\":"                       + String::format("%.2f", aX)
          + ",\"y\":"                       + String::format("%.2f", aY)
          + ",\"z\":"                       + String::format("%.2f", aZ)
          + ",\"rangeInInches\":"           + String(RangeInInches)
          + "}";
      Particle.publish("payload", sensor_payload, 60, PRIVATE);
    }
}

void locationCallback(float lat, float lon, float accuracy) {
  // if we don't have a fix from our GPS sensor yet, then use the cellular location
  if (GPS.latitude == 0) {
    cellularLatitude = lat;
    cellularLongitude = lon;
    cellularAccuracy = accuracy;
    usingCellularLocation = true;
  }
}

int crc8(String str) {
  int len = str.length();
  const char * buffer = str.c_str();

  int crc = 0;
  for(int i=0;i<len;i++) {
    crc ^= (buffer[i] & 0xff);
  }
  return crc;
}

void antennaSelect(uint8_t *buf){
    for(uint8_t i=0;i<12;i++)
    {
        Serial1.write(buf[i]); //send the command to gps module
        Serial.print(buf[i],HEX);
        Serial.print(",");
    }
    Serial.println("");
}

int sensorStatus(String command){
    publishData("Status");
    return batteryAlert;
}

void deviceNameHandler(const char *topic, const char *data) {
    Serial.println("received " + String(topic) + ": " + String(data));
    deviceName = String(data);
}
