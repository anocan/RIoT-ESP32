#include "RFID.h"
#include "Pins.h"
#include "RiotFirebase.h"
#include "RiotSystem.h"
#include <MFRC522.h>
#include <SPI.h>

MFRC522 mfrc522(SS_PIN, RST_PIN);

bool RIoTRFID::initRFID() {
  SPI.begin();
  mfrc522.PCD_Init();
  if (RIoTSystem::getInstance().SYSTEM == RIoTSystem::SYS_NORMAL) {
    digitalWrite(READY_PIN, HIGH);
  }
  return true;
}

String RIoTRFID::readRFID() {
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      Serial.print("Tag UID: ");
      String tagUID = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        tagUID += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") +
                  String(mfrc522.uid.uidByte[i], HEX);
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
      }
      Serial.println();
      mfrc522.PICC_HaltA();
      return tagUID;
    }
  }
  return "NULL";
}
