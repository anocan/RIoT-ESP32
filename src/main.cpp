#include "Network.h"
#include "Pins.h"
#include "RFID.h"
#include "RiotFirebase.h"
#include "RiotSystem.h"

void setup() {
  attachInterrupt(INTERRUPT_PIN, backUpRead, RISING);
  Serial.begin(MONITOR_SPEED);
  initWiFi();
  initFirebase();
  initRFID();
}

void loop() {
  systemMaintenance();
  doorController(readRFID());
}
