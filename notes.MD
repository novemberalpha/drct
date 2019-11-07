ISSUES / BUGS / FEATURES
===

1. Depth Sensor is enabled
1. Currently we poll google even if we have a GPS fix. We can save bandwidth and API fees by only polling Google when we don't have a GPS fix. 
1. Once we figure out a maximized power budget against useful polling rates, we'll need to determine what type of battery we should put on this thing and how long it will last without primary (asset provided) power
1. There is no API/Rules Engine to capture the data today, We're using IFTTT.com to capture data into a Google Sheet
1. We can incorporate fast-fix tech (using cell location to preheat the GPS fix acquisition) once we get other more important features done
1. We have not written the code to notify if the unit experiences a drop (zero g), collision or upset (tip over)
1. Temperature calculation/conversion is incorrect results in 42degF for 70degF
1. Remote wake capability is required
