#include "RiotFirebase.h"
#include "Pins.h"
#include "RiotSystem.h"
#include "credentials.h"

bool RIoTFirebase::initFirebase() {
  digitalWrite(FIREBASE_PIN, HIGH);
  Firebase.reconnectNetwork(true);
  Firebase.begin(&config, &auth);
  while (!Firebase.ready()) {
    if (RIoTSystem::getInstance()->SYSTEM == RIoTSystem::SYS_NORMAL) {
      Serial.print("-");
      delay(100);
    } else if (RIoTSystem::getInstance()->SYSTEM == RIoTSystem::SYS_BACKUP) {
      Serial.print("!");
      return false;
    }
  }
  if (RIoTSystem::getInstance()->SYSTEM == RIoTSystem::SYS_NORMAL) {
    digitalWrite(FIREBASE_PIN, LOW);
  }

  return true;
}

bool RIoTFirebase::firestoreGetJson(FirebaseJson *jsonObject,
                                    const char *documentPath) {
  if (WiFi.status() == WL_CONNECTED && Firebase.ready()) {
    if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "",
                                       documentPath, "")) {
      jsonObject->setJsonData(fbdo.payload().c_str());
      return true;

      // jsonObject.toString(Serial, true);
      // Serial.println(jsonObject.toString(Serial, true));
    }
    return false;
  }
  return false;
}

inline bool RIoTFirebase::firestoreUpdateField(FirebaseJson *jsonObject,
                                               const char *documentPath,
                                               const char *updateField,
                                               const char *updateValue) {
  if (WiFi.status() == WL_CONNECTED && Firebase.ready()) {
    jsonObject->set(updateField, updateValue);
    // jsonObject.toString(Serial, true);
    int firstSlash = String(updateField).indexOf('/');
    if (firstSlash != -1) {
      // Find the position of the second last '/'
      int secondSlash = String(updateField).indexOf('/', firstSlash + 1);
      if (secondSlash != -1) {
        // Extract the substring between the second last '/' and the last '/' to
        // get the update field
        String updateFieldFinal =
            String(updateField).substring(firstSlash + 1, secondSlash);
        // Serial.print("Extracted substring: ");
        // Serial.println(updateField);

        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "",
                                             documentPath, jsonObject->raw(),
                                             updateFieldFinal)) {
          return true;
          // Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
        } else {
          Serial.println("Error Patching Document!");
          return false;
        }
      }
    }
  } else {
    Serial.println(fbdo.errorReason());
    return false;
  }
  return false;
}

String RIoTFirebase::getDataFromJsonObject(FirebaseJson *jsonObject,
                                           const char *fieldPath) {
  FirebaseJsonData jsonData;

  jsonObject->get(jsonData, fieldPath, true);

  return jsonData.stringValue;
}

void RIoTFirebase::compareAndCount(const char *value1, const char *value2,
                                   int *count) {

  if (!strcmp(value1, value2)) {
    (*count)++;
  }
}

void RIoTFirebase::compareAndReturn(const char *value1, const char *value2,
                                    int *count) {

  if (strcmp(value1, value2)) {
    (*count)++;
  }
}

void RIoTFirebase::setAllInOrOutToOut(const char *riotCardID,
                                      const char *updateValue, int *count) {
  RIoTFirebase riotFirebase;
  FirebaseJson jsonRiotCard;
  char documentPath[64];
  sprintf(documentPath, "riotCards/%s", riotCardID);

  riotFirebase.firestoreGetJson(&jsonRiotCard, documentPath);
  jsonRiotCard.set("fields/inOrOut/stringValue", updateValue);

  riotFirebase.firestoreUpdateField(&jsonRiotCard, documentPath,
                                    "fields/inOrOut/stringValue", updateValue);
}

void RIoTFirebase::compareAndUpdateRiotCard(const char *riotCardID,
                                            const char *comparisonValue,
                                            int *count) {
  RIoTFirebase riotFirebase;
  FirebaseJson jsonRiotCard;
  char documentPath[64];
  sprintf(documentPath, "riotCards/%s", riotCardID);

  riotFirebase.firestoreGetJson(&jsonRiotCard, documentPath);
  String userType = riotFirebase.getDataFromJsonObject(
      &jsonRiotCard, "fields/userType/stringValue");
  if (userType != comparisonValue) {
    return;
  }

  else if (userType == comparisonValue) {
    jsonRiotCard.set("fields/riotCardStatus/stringValue", "inactive");
    riotFirebase.firestoreUpdateField(&jsonRiotCard, documentPath,
                                      "fields/riotCardStatus/stringValue",
                                      "inactive");
  }
}

String RIoTFirebase::firebaseJsonIterator(
    FirebaseJson *jsonObject, String pathToArray, const char *updateField,
    const char *funcValue,
    void (*func)(const char *, const char *, int *) = nullptr) {
  int count = 0;
  const char *info = nullptr; // I leave it to your imagination

  FirebaseJsonData jsonData;
  FirebaseJsonArray jsonArray;
  FirebaseJsonData arrayValue;
  jsonObject->get(jsonData, pathToArray);
  jsonData.getArray(jsonArray);
  char tagstr[128];

  for (size_t i = 0; i < jsonArray.size(); i++) {
    sprintf(tagstr, "/[%d]/fields/%s/stringValue", i, updateField);
    jsonArray.get(arrayValue, tagstr);
    if (func != nullptr) {
      func(arrayValue.stringValue.c_str(), funcValue, &count);
    }
    // Serial.println(arrayValue.stringValue);
  }
  if (count != 0) {
    // Serial.println(String(count));
    return String(count);
  } else if (info != nullptr) {
    return info;
  } else {
    return "NULL";
  }
}

String RIoTFirebase::getNoOfPeople() {
  int count = 0;
  int pageSize = 10;
  bool firstPage = true;
  FirebaseJson riotCardsList;
  FirebaseJsonData nextPageToken;
  const char *path = "riotCards/";

  while (riotCardsList.get(nextPageToken, "nextPageToken") || firstPage) {
    if (!firstPage) {

      // Serial.println(nextPageToken.stringValue);
      Firebase.Firestore.listDocuments(&fbdo, FIREBASE_PROJECT_ID, "", path,
                                       pageSize, nextPageToken.stringValue, "",
                                       "inOrOut", false);
      riotCardsList.setJsonData(fbdo.payload().c_str());
      String countInString = firebaseJsonIterator(
          &riotCardsList, "documents/", "inOrOut", "in", &compareAndCount);
      count += countInString.toInt();
    } else {
      Firebase.Firestore.listDocuments(&fbdo, FIREBASE_PROJECT_ID, "", path,
                                       pageSize, "", "", "inOrOut", false);
      riotCardsList.setJsonData(fbdo.payload().c_str());
      String countInString = firebaseJsonIterator(
          &riotCardsList, "documents/", "inOrOut", "in", &compareAndCount);
      count += countInString.toInt();
      firstPage = false;
    }

    // Serial.println(hope);
    // Serial.println(riotCardsList.toString(Serial, true));
  }
  // Serial.println(ESP.getFreeHeap());
  return String(count);
}

bool RIoTFirebase::updateNumberOfPeople() {
  FirebaseJson jsonObjectLabData;
  firestoreGetJson(&jsonObjectLabData, "labData/lab-data");

  firestoreUpdateField(&jsonObjectLabData, "labData/lab-data",
                       "fields/labPeople/stringValue", getNoOfPeople().c_str());

  return true;
}

bool RIoTFirebase::uploadAllFirestoreTasks(FirebaseJson *jsonObjectRiotCard,
                                           const char *riotCardID) {
  String userID =
      getDataFromJsonObject(jsonObjectRiotCard, "fields/id/stringValue");
  char riotCardPath[64];
  strcpy(riotCardPath, "riotCards/");
  strcat(riotCardPath, riotCardID);
  firestoreUpdateField(jsonObjectRiotCard, riotCardPath,
                       "fields/inOrOut/stringValue", "in");

  char userIDPath[64];
  strcpy(userIDPath, "users/");
  strcat(userIDPath, userID.c_str());
  FirebaseJson jsonObjectUser;
  firestoreGetJson(&jsonObjectUser, userIDPath);

  firestoreUpdateField(&jsonObjectUser, userIDPath,
                       "fields/riotCard/mapValue/fields/inOrOut/stringValue",
                       "in");

  updateNumberOfPeople();

  return true;
}

void RIoTFirebase::updateRiotCardStatus() {
  int pageSize = 20;
  bool firstPage = true;
  FirebaseJson riotCardsList;
  FirebaseJsonData nextPageToken;
  const char *path = "riotCards/";

  while (riotCardsList.get(nextPageToken, "nextPageToken") || firstPage) {
    if (!firstPage) {
      Firebase.Firestore.listDocuments(&fbdo, FIREBASE_PROJECT_ID, "", path,
                                       pageSize, nextPageToken.stringValue, "",
                                       "riotCardID", false);
      riotCardsList.setJsonData(fbdo.payload().c_str());

      firebaseJsonIterator(&riotCardsList, "documents/", "riotCardID",
                           "deleted", &compareAndUpdateRiotCard);

    } else {
      Firebase.Firestore.listDocuments(&fbdo, FIREBASE_PROJECT_ID, "", path,
                                       pageSize, "", "", "riotCardID", false);
      riotCardsList.setJsonData(fbdo.payload().c_str());

      firebaseJsonIterator(&riotCardsList, "documents/", "riotCardID",
                           "deleted", &compareAndUpdateRiotCard);

      firstPage = false;
    }
  }
}

void RIoTFirebase::resetInOrOutStatus() {
  int pageSize = 20;
  bool firstPage = true;
  FirebaseJson riotCardsList;
  FirebaseJsonData nextPageToken;
  const char *path = "riotCards/";

  while (riotCardsList.get(nextPageToken, "nextPageToken") || firstPage) {
    if (!firstPage) {
      Firebase.Firestore.listDocuments(&fbdo, FIREBASE_PROJECT_ID, "", path,
                                       pageSize, nextPageToken.stringValue, "",
                                       "riotCardID", false);
      riotCardsList.setJsonData(fbdo.payload().c_str());

      firebaseJsonIterator(&riotCardsList, "documents/", "riotCardID", "out",
                           &setAllInOrOutToOut);

      // Serial.println(cardObject->toString(Serial, true));
    } else {
      // Serial.println(nextPageToken.stringValue);
      Firebase.Firestore.listDocuments(&fbdo, FIREBASE_PROJECT_ID, "", path,
                                       pageSize, "", "", "riotCardID", false);
      riotCardsList.setJsonData(fbdo.payload().c_str());

      firebaseJsonIterator(&riotCardsList, "documents/", "riotCardID", "out",
                           &setAllInOrOutToOut);

      firstPage = false;
    }
  }
}