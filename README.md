DRCT V0.1 - FIRMWARE
===
This is an aggregation and update of different open source projects to create base sensor platform for CarrierDirect

HARDWARE REQUIREMENTS
===
This firmware is written for the [Particle.io](https://www.particle.io/) [Electron](https://store.particle.io/collections/cellular/products/electron-3g-americas) and [AssetTrackerV2](https://store.particle.io/collections/cellular/products/asset-tracker) Daughter Board. 
![AssetTrackerV2](https://cdn.shopify.com/s/files/1/0925/6626/products/assetTrackerV2-extra_1cd097a8-a85e-423c-8c81-89ad61db4286_1408x1408.jpg?v=1491944346)

How to use
===

Make sure you have the [particle-cli](https://github.com/spark/particle-cli) installed... 

```
# build your demo!

./build.sh drct-firmware

# then you can connect your device in USB DFU mode, and flash it with:

./flash drct-firmware
```

DRCT-CLOUD V0.1
===
This is a Prisma/Node.js/GCP implementation prototype that captures sensor data via Particle.io webhook and publishes it to a Prisma model via a simple (no auth) Node.js API hosted on GCP

How to use
===
**Install NPM - http://nodejs.org/**
`brew doctor`
`brew update`
`brew install node nvm`

**Install Prisma - https://www.prisma.io/**
`npm install -g prisma`

**Install Google Cloud CLI - **
[Google Cloud SDK](https://cloud.google.com/sdk/docs/quickstart-macos)

**Install Node.js dependencies**
`npm install`

**Install NodeMon**
`npm install -g nodemon`

You can now run express server with `nodemon server.js` and it will update the page everytime you make a change to the code. 

Update the data model by editing `datamodel.prisma` and then using `prisma deploy` from the command line. 
Deploy to GCP App Engine with `gcloud app deploy`

Enjoy!


Attributions
===

This project is possible thanks to awesome work from Particle and Adafruit, go buy stuff from them!
Thanks to David Middlecamp for assembling the demos that founded this code: https://github.com/dmiddlecamp/fancy-asset-tracker
