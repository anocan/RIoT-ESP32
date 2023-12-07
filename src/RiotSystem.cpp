#include "RiotSystem.h"
#include "Network.h"
#include "Pins.h"
#include "RFID.h"
#include "RiotFirebase.h"

const char *knownTagUIDs[] = {
    "ec2ff537",
    "e3f76c19",
    "b7918c01",
    "4c60a25f",
};

const char *correspondingIDs[] = {
    "MASTER KEY",
    "ID1",
    "ID2",
    "ZGwWrS4bjrZPXa8V2ddsES2Api33",
};

// bool RIoTSystem::taskExecuted = false;
HardwareSerial SerialPort(2);

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
  preferences.begin("RIoT", false);
  Serial.begin(MONITOR_SPEED);
  SerialPort.begin(COMM_SPEED, SERIAL_8N1, RX_PIN,
                   TX_PIN); // pins 16 rx2, 17 tx2, MONITOR_SPEED bps, 8 bits no
                            // parity 1 stop bit
  attachInterrupt(INTERRUPT_PIN, ISR_function, RISING);
  pinMode(NETWORK_PIN, OUTPUT);
  pinMode(FIREBASE_PIN, OUTPUT);
  pinMode(READY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(DOOR_PIN, OUTPUT);    // comment this line for Big Brother
  digitalWrite(DOOR_PIN, HIGH); // comment this line for Big Brother
}

void IRAM_ATTR RIoTSystem::backUpRead() {
  RIoTSystem::getInstance().resetCounter++;
  if (RIoTSystem::getInstance().resetCounter >=
      RIoTSystem::getInstance().resetThreshold) {
    // System resets after sufficient amount of trigger
    digitalWrite(NETWORK_PIN, LOW);
    digitalWrite(FIREBASE_PIN, LOW);
    digitalWrite(READY_PIN, LOW);
    RIoTSystem::getInstance().setSystemStatus(SYS_NORMAL);
    RIoTSystem::getInstance().preferences.end();
    ESP.restart();
  }

  if (RIoTSystem::getInstance().startTimer) {
    Serial.println("\nBACKUP RFID READ ACTIVATED!");
    RIoTSystem::getInstance().startTimer = false;
    RIoTSystem::getInstance().setSystemStatus(SYS_BACKUP);
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

void RIoTSystem::requestToLittleLister(const char *request) {
  SerialPort.print(request);
  // Serial.println("Door has been unlocked!"); // Communicate with Little
  if (RIoTSystem::getInstance().SYSTEM != SYS_NORMAL) {
    RIoTSystem::setSystemStatus(SYS_NORMAL);
    digitalWrite(READY_PIN, LOW);
    RIoTSystem::getInstance().preferences.end();
    ESP.restart();
  }
}

bool RIoTSystem::littleSisterDoorController() {
  if (SerialPort.available()) {
    // Serial.println(Sucessfull serial connection.);
  } else {
    // Serial.println("unsuccess");
    return false;
  }

  String requestFromBigBrother = SerialPort.readStringUntil('|');
  // Serial.println(requestFromBigBrother);
  delay(5);
  if (requestFromBigBrother == RIoTSystem::getInstance().releaseCommandBackup) {
    digitalWrite(DOOR_PIN, LOW); // Actual release
    digitalWrite(READY_PIN, LOW);
    while (true) {
      requestFromBigBrother = SerialPort.readStringUntil('|');
      Serial.println(requestFromBigBrother);
      delay(5);
      if (requestFromBigBrother ==
          RIoTSystem::getInstance().holdCommandBackup) {
        digitalWrite(DOOR_PIN, HIGH);
        digitalWrite(READY_PIN, HIGH);
        break;
      }
    }
  }
  if (WiFi.status() == WL_CONNECTED && Firebase.ready()) {

    FirebaseJson labData;
    firestoreGetJson(&labData, "labData/lab-data");
    String doorStatus =
        getDataFromJsonObject(&labData, "fields/labDoor/stringValue");

    switch (hashit(doorStatus)) {
    case DOOR_LOCKED: {
      digitalWrite(DOOR_PIN, HIGH);
      digitalWrite(NETWORK_PIN, LOW);
      digitalWrite(FIREBASE_PIN, LOW);
      if (requestFromBigBrother == RIoTSystem::getInstance().releaseCommand) {
        digitalWrite(DOOR_PIN, LOW); // Actual release
        digitalWrite(READY_PIN, LOW);
        while (true) {
          requestFromBigBrother = SerialPort.readStringUntil('|');
          delay(5);
          if (requestFromBigBrother == RIoTSystem::getInstance().holdCommand) {
            digitalWrite(DOOR_PIN, HIGH);
            digitalWrite(READY_PIN, HIGH);

            break;
          }
        }
        return true;
      }

      break;
    }

    case DOOR_UNLOCKED: {
      digitalWrite(DOOR_PIN, LOW);
      digitalWrite(NETWORK_PIN, LOW);
      digitalWrite(FIREBASE_PIN, LOW);
      digitalWrite(READY_PIN, LOW);
      delay(500);
      digitalWrite(READY_PIN, HIGH);
      break;
    }

    case DOOR_SECURED: {
      digitalWrite(DOOR_PIN, HIGH);
      digitalWrite(NETWORK_PIN, HIGH);
      digitalWrite(FIREBASE_PIN, HIGH);
      digitalWrite(READY_PIN, HIGH);
      if (requestFromBigBrother == RIoTSystem::getInstance().releaseCommand) {
        digitalWrite(DOOR_PIN, LOW); // Actual release
        digitalWrite(NETWORK_PIN, LOW);
        digitalWrite(FIREBASE_PIN, LOW);
        digitalWrite(READY_PIN, LOW);
        while (true) {
          requestFromBigBrother = SerialPort.readStringUntil('|');
          if (requestFromBigBrother == RIoTSystem::getInstance().holdCommand) {
            digitalWrite(DOOR_PIN, HIGH);
            digitalWrite(NETWORK_PIN, HIGH);
            digitalWrite(FIREBASE_PIN, HIGH);
            digitalWrite(READY_PIN, HIGH);

            break;
          }
        }
        return true;
      }

      break;
    }

    default: {
      return false;
      break;
    }
    }
  } else {
    digitalWrite(NETWORK_PIN, LOW);
    digitalWrite(FIREBASE_PIN, LOW);
    if (requestFromBigBrother == RIoTSystem::getInstance().releaseCommand) {
      digitalWrite(DOOR_PIN, LOW); // Actual release
      digitalWrite(READY_PIN, LOW);
      while (true) {
        requestFromBigBrother = SerialPort.readStringUntil('|');
        delay(5);
        if (requestFromBigBrother == RIoTSystem::getInstance().holdCommand) {
          digitalWrite(DOOR_PIN, HIGH);
          digitalWrite(READY_PIN, HIGH);
          break;
        }
      }
      return true;
    }
  }
  return false;
}

void RIoTSystem::bigBrotherDoorController(String tagUID) {
  doorHoldStartTime = millis(); // Record the start time
  if (RIoTSystem::getInstance().SYSTEM == SYS_NORMAL) {
    if (tagUID == "NULL") {
      // Serial.println("No RFID read."); // do not uncomment this -spam
      return;
    }

    char riotCardPath[64];
    strcpy(riotCardPath, "riotCards/");
    strcat(riotCardPath, tagUID.c_str());
    FirebaseJson jsonObjectRiotCard;
    FirebaseJson jsonObjectDoor;
    if (firestoreGetJson(&jsonObjectRiotCard, riotCardPath)) {
      logger(&jsonObjectRiotCard, tagUID);
      // Serial.println("Card read successfuly")
    } else {
      beep(buzzerWrongDuration);
      logger(&jsonObjectRiotCard, tagUID);
      return;
    }
    firestoreGetJson(&jsonObjectDoor, "labData/lab-data");

    String dataDoor =
        getDataFromJsonObject(&jsonObjectDoor, "fields/labDoor/stringValue");

    switch (hashit(dataDoor)) {
    case DOOR_LOCKED: {
      String jsonDataRiotCardStatus = getDataFromJsonObject(
          &jsonObjectRiotCard, "fields/riotCardStatus/stringValue");
      if (jsonDataRiotCardStatus == "active") {
        digitalWrite(READY_PIN, LOW);
        beep(buzzerCorrectDuration);
        requestToLittleLister(releaseCommand);
        uploadAllFirestoreTasks(&jsonObjectRiotCard, tagUID.c_str());
        while (millis() - doorHoldStartTime <= doorHoldDuration) {
          // Serial.println("waiting to lock...");
        }
        requestToLittleLister(holdCommand);
        digitalWrite(READY_PIN, HIGH);
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
      Serial.println("RIoT door is already unlocked.");
      break;
    }

    case DOOR_SECURED: {
      String userType = getDataFromJsonObject(&jsonObjectRiotCard,
                                              "fields/userType/stringValue");
      if (userType == "admin" || userType == "superadmin") {
        digitalWrite(READY_PIN, LOW);
        beep(buzzerCorrectDuration);
        requestToLittleLister(releaseCommand);
        while (millis() - doorHoldStartTime <= doorHoldDuration) {
          Serial.println("waiting to lock...");
        }
        requestToLittleLister(holdCommand);
        digitalWrite(READY_PIN, HIGH);
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
    if (tagUID == "NULL") {
      // Serial.println("No RFID read."); // do not uncomment this -spam
      return;
    }
    const int numKnownTags = sizeof(knownTagUIDs) / sizeof(knownTagUIDs[0]);
    for (int i = 0; i < numKnownTags; i++) {
      if (tagUID == knownTagUIDs[i]) {
        // Match found
        digitalWrite(READY_PIN, LOW);
        digitalWrite(NETWORK_PIN, LOW);
        digitalWrite(FIREBASE_PIN, LOW);
        beep(buzzerCorrectDuration);
        requestToLittleLister(releaseCommand);
        while (millis() - doorHoldStartTime <= doorHoldDuration) {
          Serial.println("waiting to lock...");
        }
        requestToLittleLister(holdCommand);
        digitalWrite(READY_PIN, HIGH);
        Serial.println("CARD READ VIA BACKUP");
        break;
      }
    }
    beep(buzzerWrongDuration);

    return;
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
    if (!getLocalTime(&timeinfo, 1000)) {
      Serial.println("Failed to obtain time");
      return false;
    }
    // Check if it's between maintenanceLower Time and maintenanceUpper Time
    if ((timeinfo.tm_hour > maintenanceLowerHour ||
         (timeinfo.tm_hour == maintenanceLowerHour &&
          timeinfo.tm_min >= maintenanceLowerMinute)) &&
        (timeinfo.tm_hour < maintenanceUpperHour ||
         (timeinfo.tm_hour == maintenanceUpperHour &&
          timeinfo.tm_min <= maintenanceUpperMinute))) {
      if (!taskExecuted) {
        Serial.println("SYSTEM MAINTENANCE!");
        // Serial.println(ESP.getFreeHeap());
        createLogDocument(&timeinfo);
        Serial.println("25% ---------- New Log document is created.");
        digitalWrite(NETWORK_PIN, HIGH);
        digitalWrite(READY_PIN, LOW);
        updateRiotCardStatus();
        Serial.println("50% ---------- RIoT Card status are updated.");
        digitalWrite(FIREBASE_PIN, HIGH);
        resetInOrOutStatus();
        Serial.println("75% ---------- InOrOut status are updated.");
        digitalWrite(READY_PIN, HIGH);
        updateNumberOfPeople();
        Serial.println(
            "100% ---------- Number of people in the lab is updated.");
        digitalWrite(NETWORK_PIN, LOW);
        digitalWrite(FIREBASE_PIN, LOW);
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
