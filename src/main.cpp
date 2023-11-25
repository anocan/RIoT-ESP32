#include "RiotSystem.h"
#include "Network.h"
#include "RiotFirebase.h"
#include "RFID.h"
#include "Pins.h"

void setup() {
    attachInterrupt(INTERRUPT_PIN, backUpRead, RISING);
    Serial.begin(monitor_speed);
    initWiFi();
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    initFirebase();
    initRFID();
}

void loop() {
    systemMaintenance();
    doorController(readRFID());
}
