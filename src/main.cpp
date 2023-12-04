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
  riotNetwork.initWiFi();
  initFirebase(); // riotFirebase.initFirebase();
  riotRfid.initRFID();
}

void loop() {
  riotSystem.bigBrotherDoorController(riotRfid.readRFID());
  riotSystem.littleSisterDoorController();
  // riotSystem.systemMaintenance();
}
