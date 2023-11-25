#include "RFID.h"
#include "RiotFirebase.h"
#include "RiotSystem.h"
#include <SPI.h>
#include <MFRC522.h>
#include "Pins.h"


MFRC522 mfrc522(SS_PIN, RST_PIN);

void initRFID() {
    if (SYSTEM == SYS_NORMAL) {
        pinMode(READY_PIN, OUTPUT);
    }
    SPI.begin();
    mfrc522.PCD_Init();
    if (SYSTEM == SYS_NORMAL) {
        digitalWrite(READY_PIN, HIGH);
    }

}   

String readRFID() {
    if (mfrc522.PICC_IsNewCardPresent()) {
        if (mfrc522.PICC_ReadCardSerial()) {
            Serial.print("Tag UID: ");
            String tagUID = "";
            for (byte i = 0; i < mfrc522.uid.size; i++) {
                tagUID += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") + String(mfrc522.uid.uidByte[i], HEX);
                Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                Serial.print(mfrc522.uid.uidByte[i], HEX);
            }
            Serial.println();
            mfrc522.PICC_HaltA();
            return tagUID;
        }
    }
    return "null";
}

