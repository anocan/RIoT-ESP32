#include "Network.h"
#include "Pins.h"
#include "RFID.h"
#include "RiotFirebase.h"
#include "RiotSystem.h"

RIoTNetwork riotNetwork;
RIoTRFID riotRfid;
RIoTSystem &riotSystem = RIoTSystem::getInstance();
// RIoTFirebase riotFirebase;

void setup() {
  riotSystem.setUpPins();
  // riotNetwork.initWiFi();
  //  initFirebase(); // riotFirebase.initFirebase();
  //  iotRfid.initRFID();
}

void loop() {
  // riotSystem.bigBrotherDoorController(riotRfid.readRFID());
  riotSystem.requestToLittleLister("ANAN");
  // riotSystem.systemMaintenance();
}
