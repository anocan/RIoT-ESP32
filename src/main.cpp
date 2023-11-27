#include "Network.h"
#include "Pins.h"
#include "RFID.h"
#include "RiotFirebase.h"
#include "RiotSystem.h"

RIoTNetwork riotNetwork;
RIoTFirebase riotFirebase;
RIoTSystem riotSystem;
RIoTRFID riotRfid;

void setup() {
  riotSystem.setUpPins();
  riotNetwork.initWiFi();
  riotFirebase.initFirebase();
  riotRfid.initRFID();
}

void loop() {
  riotSystem.systemMaintenance();
  riotSystem.doorController(riotRfid.readRFID());
}
