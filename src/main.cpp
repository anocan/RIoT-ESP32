#include "Network.h"
#include "Pins.h"
#include "RFID.h"
#include "RiotFirebase.h"
#include "RiotSystem.h"

void setup() {
  setUpPins();
  initWiFi();
  initFirebase();
  initRFID();
}

void loop() {
  systemMaintenance();
  doorController(readRFID());
}
