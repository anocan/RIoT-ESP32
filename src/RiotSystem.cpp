#include "RiotSystem.h"
#include "Network.h"
#include "RiotFirebase.h"
#include "RFID.h"
#include "Pins.h"

SYSTEM_STATUS SYSTEM = SYS_NORMAL;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3*3600; // +3 UTC in seconds
const int   daylightOffset_sec = 0;

const int maintenanceLowerHour = 04;
const int maintenanceLowerMinute = 00;

const int maintenanceUpperHour = 04;
const int maintenanceUpperMinute = 59;

bool taskExecuted = false;

boolean startTimer = true;
long now = micros();
long lastTrigger = 0;
int resetCounter = 0;
int resetThreshold = 5;

const char* knownTagUIDs[] = {
    "ec2ff537",
    "e3f76c19",
    "4c60a25f"
};

const int numKnownTags = sizeof(knownTagUIDs) / sizeof(knownTagUIDs[0]);
const char* correspondingIDs[] = {
    "MASTER KEY",
    "ID1",
    "ZGwWrS4bjrZPXa8V2ddsES2Api33"
};

void IRAM_ATTR backUpRead() {
  startTimer = true;
  resetCounter++;
  if (resetCounter >= resetThreshold) {
    // System resets after sufficient amount of trigger
    ESP.restart();
  }

    if(startTimer && (now - lastTrigger > (INTERRUPT_SECONDS*1000000))) {
    pinMode(NETWORK_PIN, OUTPUT);
    pinMode(FIREBASE_PIN, OUTPUT); 
    pinMode(READY_PIN, OUTPUT);   
    Serial.println("BACKUP RFID READ ACTIVATED");
    lastTrigger = micros();
    startTimer = false;
    SYSTEM = SYS_BACKUP;
    digitalWrite(NETWORK_PIN,HIGH);
    digitalWrite(FIREBASE_PIN,HIGH);
    digitalWrite(READY_PIN,HIGH);
  }
}

void releaseDoor() {
  Serial.println("Door has been unlocked!");
}

void doorController(String tagUID) {
  if (tagUID == "null") {
    //Serial.println("No RFID read."); // do not uncomment this -spam
    return;
  }
      //Serial.println(tagUID);
          if (SYSTEM == SYS_NORMAL) {
            char riotCardPath[64];
            strcpy(riotCardPath, "riotCards/");
            strcat(riotCardPath, tagUID.c_str());
             FirebaseJson jsonObjectRiotCard;
             FirebaseJson jsonObjectDoor;
              firestoreGetJson(&jsonObjectRiotCard , riotCardPath);
              firestoreGetJson(&jsonObjectDoor, "labData/lab-data");
             

             String dataDoor = getDataFromJsonObject(&jsonObjectDoor,
             "fields/labDoor/stringValue"
             );

             //Serial.println(dataDoor);
            if (dataDoor == "locked") {


                String jsonDataRiotCardStatus = getDataFromJsonObject(&jsonObjectRiotCard, 
                "fields/riotCardStatus/stringValue");
                //Serial.println(jsonDataRiotCardStatus);
                
                if (jsonDataRiotCardStatus == "active") {
                  releaseDoor();
                  uploadAllFirestoreTasks(&jsonObjectRiotCard, tagUID.c_str());
                    
                } else if (jsonDataRiotCardStatus == "inactive") {
                  Serial.println("Card is inactive."); // DEBUG
                } else if (jsonDataRiotCardStatus == "disabled") {
                  Serial.println("Card is disabled by the admin."); // DEBUG
                }

            } else if (dataDoor == "unlocked") {
                releaseDoor();
                Serial.println("RIoT door is already unlocked.");
            } else if (dataDoor == "secured") {
                String userType = getDataFromJsonObject(&jsonObjectRiotCard, "fields/userType/stringValue");
                if (userType == "admin" || userType == "superadmin") {
                    releaseDoor();
                    Serial.println("vvvv");
                }
            }


        } else if (SYSTEM == SYS_BACKUP) {
            for (int i = 0; i < numKnownTags; i++) {
                if (tagUID == knownTagUIDs[i]) {
                    // Match found

                    /// UNLOCK THE DOOR
                   Serial.println("CARD READ VIA BACKUP");
                    break; // Exit the loop when a match is found
                }
            }
    
        }
}

void systemMaintenance(){
  time_t now;
  time(&now);
  struct tm timeinfo;
  //char formattedTime[30]; // Buffer to hold the formatted time

  localtime_r(&now, &timeinfo);

  // Get current time
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

    // Check if it's between maintenanceLower Time and maintenanceUpper Time
  if (timeinfo.tm_hour == maintenanceLowerHour && timeinfo.tm_min >= maintenanceLowerMinute && timeinfo.tm_hour == maintenanceUpperHour && timeinfo.tm_min <= maintenanceUpperMinute) {
    if (!taskExecuted) {
      Serial.println("SYSTEM MAINTENANCE!");
      updateRiotCardStatus();
      Serial.println("33% ---------- RIoT Card status are updated.");
      resetInOrOutStatus();
      Serial.println("67% ---------- InOrOut status are updated.");
      updateNumberOfPeople();
      Serial.println("100% ---------- Number of people in the lab is updated.");
      taskExecuted = true;
    }
  } else {
    taskExecuted = false; // Reset the flag if not within the specified time
  }

  delay(1000); // Delay to avoid excessive checking
  // Format time using strftime
  // strftime(formattedTime, sizeof(formattedTime), "%A, %B %d %Y %H:%M:%S", &timeinfo);
    //Serial.println(formattedTime);

}