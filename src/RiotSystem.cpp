#include "RiotSystem.h"
#include "Network.h"
#include "Pins.h"
#include "RFID.h"
#include "RiotFirebase.h"

const char *knownTagUIDs[] = {
    "ec2ff537",
    "e3f76c19",
    "4c60a25f",
};

const char *correspondingIDs[] = {
    "MASTER KEY",
    "ID1",
    "ZGwWrS4bjrZPXa8V2ddsES2Api33",
};

bool RIoTSystem::taskExecuted = false;

RIoTSystem::SYSTEM_STATUS RIoTSystem::SYSTEM = RIoTSystem::SYS_NORMAL;

RIoTSystem::DOOR_STATUS RIoTSystem::hashit(String string) {
  if (string == "locked")
    return DOOR_LOCKED;
  else if (string == "unlocked")
    return DOOR_UNLOCKED;
  else if (string == "secured")
    return DOOR_SECURED;
  else
    return DOOR_DEFAULT;
}

void RIoTSystem::setUpPins() {
  Serial.begin(MONITOR_SPEED);
  // attachInterrupt(INTERRUPT_PIN, backUpReadWrapper, RISING);
  pinMode(NETWORK_PIN, OUTPUT);
  pinMode(FIREBASE_PIN, OUTPUT);
  pinMode(READY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
}

void IRAM_ATTR RIoTSystem::backUpRead() {
  resetCounter++;
  if (resetCounter >= resetThreshold) {
    // System resets after sufficient amount of trigger
    digitalWrite(NETWORK_PIN, LOW);
    digitalWrite(FIREBASE_PIN, LOW);
    digitalWrite(READY_PIN, LOW);
    RIoTSystem::setSystemStatus(SYS_NORMAL);
    ESP.restart();
  }

  if (startTimer) {
    Serial.println("\nBACKUP RFID READ ACTIVATED!");
    startTimer = false;
    RIoTSystem::setSystemStatus(SYS_BACKUP);
    digitalWrite(NETWORK_PIN, HIGH);
    digitalWrite(FIREBASE_PIN, HIGH);
    digitalWrite(READY_PIN, HIGH);
  }
}

void RIoTSystem::beep(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

void RIoTSystem::releaseDoor() {
  RIoTFirebase RIoTFirebase;
  digitalWrite(READY_PIN, LOW);
  beep(200);
  digitalWrite(READY_PIN, HIGH);
  Serial.println("Door has been unlocked!"); // ACTUAL RELEASE
  riotFirebase.updateRiotCardStatus();
  if (RIoTSystem::getInstance().SYSTEM != SYS_NORMAL) {
    RIoTSystem::setSystemStatus(SYS_NORMAL);
    digitalWrite(READY_PIN, LOW);
    ESP.restart();
  }
}

void RIoTSystem::doorController(String tagUID) {
  if (RIoTSystem::getInstance().SYSTEM == SYS_NORMAL) {
    if (tagUID == "NULL") {
      // Serial.println("No RFID read."); // do not uncomment this -spam
      return;
    }
    RIoTFirebase riotFirebase;
    char riotCardPath[64];
    strcpy(riotCardPath, "riotCards/");
    strcat(riotCardPath, tagUID.c_str());
    FirebaseJson jsonObjectRiotCard;
    FirebaseJson jsonObjectDoor;
    riotFirebase.firestoreGetJson(&jsonObjectRiotCard, riotCardPath);
    riotFirebase.firestoreGetJson(&jsonObjectDoor, "labData/lab-data");

    String dataDoor = riotFirebase.getDataFromJsonObject(
        &jsonObjectDoor, "fields/labDoor/stringValue");

    switch (hashit(dataDoor)) {
    case DOOR_LOCKED: {
      String jsonDataRiotCardStatus = riotFirebase.getDataFromJsonObject(
          &jsonObjectRiotCard, "fields/riotCardStatus/stringValue");
      if (jsonDataRiotCardStatus == "active") {
        releaseDoor();
        riotFirebase.uploadAllFirestoreTasks(&jsonObjectRiotCard,
                                             tagUID.c_str());
      } else if (jsonDataRiotCardStatus == "inactive") {
        digitalWrite(NETWORK_PIN, HIGH);
        beep(buzzerWrongDuration);
        digitalWrite(NETWORK_PIN, LOW);

        Serial.println("Card is inactive."); // DEBUG
      } else if (jsonDataRiotCardStatus == "disabled") {
        digitalWrite(FIREBASE_PIN, HIGH);
        beep(buzzerWrongDuration);
        digitalWrite(FIREBASE_PIN, LOW);
        Serial.println("Card is disabled by the admin."); // DEBUG
      }
      break;
    }

    case DOOR_UNLOCKED: {
      releaseDoor();
      Serial.println("RIoT door is already unlocked.");
      break;
    }

    case DOOR_SECURED: {
      String userType = riotFirebase.getDataFromJsonObject(
          &jsonObjectRiotCard, "fields/userType/stringValue");
      if (userType == "admin" || userType == "superadmin") {
        releaseDoor();
        Serial.println("Admin pass.");
      } else {
        digitalWrite(NETWORK_PIN, HIGH);
        digitalWrite(FIREBASE_PIN, HIGH);
        beep(buzzerWrongDuration);
        digitalWrite(NETWORK_PIN, LOW);
        digitalWrite(FIREBASE_PIN, LOW);
        Serial.println("Unauthorized access.");
      }
      break;
    }

    default: {
      break;
    }
    }
  } else if (RIoTSystem::getInstance().SYSTEM == SYS_BACKUP) {
    const int numKnownTags = sizeof(knownTagUIDs) / sizeof(knownTagUIDs[0]);
    for (int i = 0; i < numKnownTags; i++) {
      if (tagUID == knownTagUIDs[i]) {
        // Match found
        digitalWrite(NETWORK_PIN, LOW);
        digitalWrite(FIREBASE_PIN, LOW);
        releaseDoor();
        Serial.println("CARD READ VIA BACKUP");
        break;
      }
    }
  }
}

bool RIoTSystem::systemMaintenance() {
  if (RIoTSystem::getInstance().SYSTEM == SYS_NORMAL) {
    time_t now;
    time(&now);
    struct tm timeinfo;
    // char formattedTime[30]; // Buffer to hold the formatted time

    localtime_r(&now, &timeinfo);

    // Get current time
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return false;
    }
    // Check if it's between maintenanceLower Time and maintenanceUpper Time
    if (timeinfo.tm_hour == maintenanceLowerHour &&
        timeinfo.tm_min >= maintenanceLowerMinute &&
        timeinfo.tm_hour == maintenanceUpperHour &&
        timeinfo.tm_min <= maintenanceUpperMinute) {
      Serial.println(taskExecuted);
      if (!taskExecuted) {
        RIoTFirebase riotFirebase;
        Serial.println("SYSTEM MAINTENANCE!");
        Serial.println(ESP.getFreeHeap());
        riotFirebase.updateRiotCardStatus();
        Serial.println("33% ---------- RIoT Card status are updated.");
        // Serial.println(riotFirebase.updateNumberOfPeople());
        Serial.println("67% ---------- InOrOut status are updated.");
        // Serial.println(riotFirebase.updateNumberOfPeople());
        //  riotFirebase.updateNumberOfPeople();
        Serial.println(
            "100% ---------- Number of people in the lab is updated.");
        taskExecuted = true;
        return true;
      }
    } else {
      taskExecuted = false; // Reset the flag if not within the specified time
    }
    delay(1000); // Delay to avoid excessive checking
  } else if (RIoTSystem::getInstance().SYSTEM == SYS_BACKUP) {
    return false;
  }

  return false;
}