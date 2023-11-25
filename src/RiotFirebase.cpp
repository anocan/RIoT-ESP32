#include "RiotFirebase.h"
#include "credentials.h"
#include "RiotSystem.h"
#include "Pins.h"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

    /**
     * Initialize the Firebase
     *
     * @return none
     *
     */
int initFirebase() {
    pinMode(FIREBASE_PIN, OUTPUT);
    digitalWrite(FIREBASE_PIN, HIGH);
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    Firebase.reconnectNetwork(true);
    fbdo.setBSSLBufferSize(4096, 1024);
    fbdo.setResponseSize(2048);
    Firebase.begin(&config, &auth);
    while (!Firebase.ready()) {
        if (SYSTEM == SYS_NORMAL) {
            Serial.print("-");
            delay(100);
        } else if (SYSTEM == SYS_BACKUP) {
            return -1;
        }
    }
    if (SYSTEM == SYS_NORMAL) {
        digitalWrite(FIREBASE_PIN, LOW);
    }

    return 1;
}

    /**
     * Get active RIoT card ids.
     *
     * @param jsonObject Json object to get the active RIoT card id
     * @return nothing
     *
     */
/* LEGACY
const char* getActiveRiotCardIDs(FirebaseJson jsonObject) {
    String activeTagUIDS = "";
    const char bookmark = '|';

    FirebaseJsonData iterator;
    char fieldPath[256];
    strcpy(fieldPath, "fields/riotCardList/arrayValue/values/[0]/mapValue/fields/id/stringValue");
    int i =0;
    while(jsonObject.get(iterator,
    fieldPath, 
    true)){
        const char* riotCardStatusBool = firestoreCompare(
        jsonObject,
        ("fields/riotCardList/arrayValue/values/[" + String(i) + "]/mapValue/fields/riotCardStatus/stringValue").c_str(),
        "active",
        "none",
        false
        );
        //Serial.println(riotCardStatusBool);
        if (!strcmp(riotCardStatusBool, "true")) {
            const char* riotCardID = getDataFromJsonObject(
            jsonObject, 
            ("fields/riotCardList/arrayValue/values/[" + String(i)  + "]/mapValue/fields/riotCardID/stringValue").c_str()
            ).c_str();
            activeTagUIDS += riotCardID + bookmark;
            delete[] riotCardID;
        }
                memset(fieldPath, 0, sizeof(fieldPath));
                strcpy(fieldPath, "fields/riotCardList/arrayValue/values");
                strcat(fieldPath, "/[");
                strcat(fieldPath, String(i).c_str());
                strcat(fieldPath, "]/");
                strcat(fieldPath, "mapValue/fields/id/stringValue");
        i++;
    }

    char* activeTagUIDs = new char[activeTagUIDS.length() + 1];
    strcpy(activeTagUIDs, activeTagUIDS.c_str());
    //Serial.println(activeTagUIDs);
    return activeTagUIDs;
} 
*/

    /**
     * Get FirebaseJson object from FirebaseFirestore.
     *
     * @param documentPath The document path to the Firestore, e.g. "riotCards/riot-cards"
     * @return FirebaseJson data
     *
     */
    inline bool firestoreGetJson(FirebaseJson* jsonObject, const char* documentPath) {
        if (WiFi.status() == WL_CONNECTED && Firebase.ready()) {
       if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath, "")) {
        jsonObject->setJsonData(fbdo.payload().c_str());
        return true;

        //jsonObject.toString(Serial, true);
        //Serial.println(jsonObject.toString(Serial, true));
    }
    return false;
    }
    return false;
}

    /**
     * Update FirebaseFirestore using FirebaseJson.
     *
     * @param jsonObject Raw json data before any change.
     * @param documentPath The document path to the Firestore, e.g. "riotCards/riot-cards"
     * @param updateWhere Path to the update field, e.g. "fields/riotCardList/arrayValue/values/[0]/mapValue/fields/inOrOut/stringValue"
     * @param updateValue Update value 
     * @return nothing
     *
     */
void firestoreUpdateField(FirebaseJson jsonObject, const char* documentPath, const char* updateWhere, const char* updateValue) {
         if (WiFi.status() == WL_CONNECTED && Firebase.ready()) {
            jsonObject.set(updateWhere, updateValue);
            //jsonObject.toString(Serial, true);
               int firstSlash = String(updateWhere).indexOf('/');
               if (firstSlash != -1) {
                // Find the position of the second last '/'
                int secondSlash = String(updateWhere).indexOf('/', firstSlash + 1);
                if (secondSlash != -1) {
                // Extract the substring between the second last '/' and the last '/' to get the update field
                String updateField = String(updateWhere).substring(firstSlash + 1, secondSlash);
                //Serial.print("Extracted substring: ");
                //Serial.println(updateField);
               


    if(Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath, jsonObject.raw(), updateField)){
            //Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
                } else {
                    Serial.println("Error Patching Document!");
                }
                }
            
        }           
    }else{
      Serial.println(fbdo.errorReason());
    }
}

    /**
     * Comperate the compareValue in the compareField, set iteration path if compareField is an array
     *
     * @param jsonObject Json data to compare
     * @param compareField Field to compared to e.g. "fields/riotCardList/arrayValue/values"
     * @param compareValue Value to be compared e.g. "riotCardID"
     * @param iteration If iteration is needed remain path to compareField should be satisfied in this e.g. "mapValue/fields/riotCardID/stringValue"
     * @param count Count is default set to false, if number of correct matches is needed set this to true
     * @return Function returns the index of the array if iteration is set with count is set to false, 
     * number of correct matches if iteration and count is set, or bool value of the comparison if iteration is unset
     *
     */
/* LEGACY
const char* firestoreCompare(FirebaseJson jsonObject, const char* compareField, const char* compareValue, const char* iteration = "none", bool count = false) {
        FirebaseJsonData jsonData;


        if (strcmp(iteration, "none")) {
            if (count != false) {
                char compareFieldPath[256];
                strcpy(compareFieldPath, compareField);
                strcat(compareFieldPath, "/[0]/");
                strcat(compareFieldPath, iteration);

                int i = 0;
                int count = 0;
                while (jsonObject.get(jsonData,
                compareFieldPath, 
                true)) {
                    Serial.println(compareFieldPath);
                if (!strcmp(compareValue, jsonData.stringValue.c_str())) {
                    count++;
                }
                memset(compareFieldPath, 0, sizeof(compareFieldPath));
                strcpy(compareFieldPath, compareField);
                strcat(compareFieldPath, "/[");
                strcat(compareFieldPath, String(i).c_str());
                strcat(compareFieldPath, "]/");
                strcat(compareFieldPath, iteration);
                i++;
                }
                return String(count).c_str();
            } else {
                char compareFieldPath[256];
                strcpy(compareFieldPath, compareField);
                strcat(compareFieldPath, "/[0]/");
                strcat(compareFieldPath, iteration);
                int i = 0;
                while (jsonObject.get(jsonData,
                compareFieldPath, 
                true)) {
                    if (!strcmp(compareValue, jsonData.stringValue.c_str())) {
                        return String(i).c_str();
                    }
                    memset(compareFieldPath, 0, sizeof(compareFieldPath));
                    strcpy(compareFieldPath, compareField);
                    strcat(compareFieldPath, "/[");
                    strcat(compareFieldPath, String(i).c_str());
                    strcat(compareFieldPath, "]/");
                    strcat(compareFieldPath, iteration);
                    i++;
                }
            }     
        } else {
            jsonObject.get(jsonData,
            compareField,
            true
            );
            if (!strcmp(compareValue, jsonData.stringValue.c_str())) {
                return "true";
            }
        }
    return "false";
}
*/

    /**
     * Comperate the compareValue in the compareField, set iteration path if compareField is an array
     *
     * @param jsonObject The raw json data to be processed
     * @param fieldPath Path to the Firestore element to be returned
     * @return Value corresponding to the field
     *
     */
String getDataFromJsonObject(FirebaseJson jsonObject, const char* fieldPath) {
    FirebaseJsonData jsonData;

    jsonObject.get(jsonData, fieldPath, true);

    return jsonData.stringValue;
}

void compareAndCount(const char* value1, const char* value2, int* count) {

    if (!strcmp(value1, value2)) {
        (*count)++;
    }
}

void compareAndReturn(const char* value1, const char* value2, int* count) {

    if (strcmp(value1, value2)) {
        (*count)++;
    }
}

/*
void setAllInOrOutToOut(const char* riotCardID, const char* updateValue, int* count) {
    char* documentPath = (char*)malloc(64 * sizeof(char));
    sprintf(documentPath, "riotCards/%s", riotCardID);
    //jsonRiotCards.set("fields/inOrOut/stringValue", updateValue);
    //Serial.println(firestoreGetJson(documentPath).toString(Serial, true));
    Serial.println(ESP.getFreeHeap());

    
    firestoreUpdateField(
        firestoreGetJson(documentPath),
        documentPath,
        "fields/inOrOut/stringValue",
        updateValue
    );
}
*/

String firebaseJsonIterator(FirebaseJson jsonObject, String pathToArray, const char* updateField, const char* funcValue, void (*func) (const char*, const char*, int*) = nullptr) {
    int count = 0;
    const char* info = nullptr; // I leave it to your imagination

    FirebaseJsonData jsonData;
    FirebaseJsonArray jsonArray;
    FirebaseJsonData arrayValue;
    jsonObject.get(jsonData, pathToArray);
    jsonData.getArray(jsonArray);
    //Serial.println(jsonArray.size());
     char tagstr[128];
    //Serial.println("b");
       for (size_t i = 0; i < jsonArray.size(); i++) {
        //Serial.println("c");
        //Serial.println(i);
      sprintf(tagstr, "/[%d]/fields/%s/stringValue", i, updateField);
      jsonArray.get(arrayValue, tagstr);
      if (func != nullptr) {
        //Serial.println("d");
      func(arrayValue.stringValue.c_str(), funcValue, &count);
      }
      //Serial.println(arrayValue.stringValue);
      
    }
    if (count != 0) {
        //Serial.println(String(count));
        return String(count);
    } else if (info != nullptr) {
        return info;
    } else {
        return "NULL";
    }
}

String getNoOfPeople() {
    int count = 0;
    int pageSize = 10;
    bool firstPage = true;
    FirebaseJson riotCardsList;
    FirebaseJsonData nextPageToken;
    const char* path = "riotCards/";
    //Serial.println(ESP.getFreeHeap());

    while(riotCardsList.get(nextPageToken,
    "nextPageToken" 
    ) || firstPage ) {
    if (!firstPage) {
        Serial.println(ESP.getFreeHeap());

        //Serial.println(nextPageToken.stringValue);
        Firebase.Firestore.listDocuments(&fbdo, FIREBASE_PROJECT_ID, "", path, pageSize, nextPageToken.stringValue, "", "inOrOut", false);
        riotCardsList.setJsonData(fbdo.payload().c_str());
        String countInString = firebaseJsonIterator(riotCardsList,
        "documents/",
        "inOrOut",
        "in",
        &compareAndCount);    
        count += countInString.toInt();
    } else {
        Firebase.Firestore.listDocuments(&fbdo, FIREBASE_PROJECT_ID, "", path, pageSize, "", "", "inOrOut", false);
        riotCardsList.setJsonData(fbdo.payload().c_str());
         String countInString = firebaseJsonIterator(riotCardsList,
        "documents/",
        "inOrOut",
        "in",
        &compareAndCount);
        count += countInString.toInt();       
        firstPage = false;
    }
    

    //Serial.println(hope);
    //Serial.println(riotCardsList.toString(Serial, true));
    } 
    //Serial.println(ESP.getFreeHeap());
    return String(count);
}

    /**
     * Upload and complete all FirebaseFirestore related tasks
     *
     * @param jsonObjectRiotCard RIoT card's json object
     * @param riotCardID RIoT card id
     * @return none
     *
     */
void uploadAllFirestoreTasks(FirebaseJson jsonObjectRiotCard, const char* riotCardID) {
    FirebaseJson jsonObjectLabData;
    firestoreGetJson(&jsonObjectLabData ,"labData/lab-data");
    String userID = getDataFromJsonObject(jsonObjectRiotCard, 
    "fields/id/stringValue"
    ); 
            char riotCardPath[64];
            strcpy(riotCardPath, "riotCards/");
            strcat(riotCardPath, riotCardID);
                firestoreUpdateField(
                jsonObjectRiotCard, 
                riotCardPath, 
                "fields/inOrOut/stringValue",
                "in"
                );

                char userIDPath[64];
                strcpy(userIDPath, "users/");
                strcat(userIDPath, userID.c_str());
                FirebaseJson jsonObjectUser;
                firestoreGetJson(&jsonObjectUser, userIDPath); 

                firestoreUpdateField(
                jsonObjectUser,
                userIDPath,  
                "fields/riotCard/mapValue/fields/inOrOut/stringValue",
                "in"
                );

                String noPeopleInTheLab = getNoOfPeople();
                firestoreUpdateField(
                jsonObjectLabData, 
                "labData/lab-data", 
                "fields/labPeople/stringValue",
                noPeopleInTheLab.c_str()
                );

}
    /**
     * Updates card status when user account is deleted
     *
      * @return none
     *
     */
/* LEGACY
void changeRiotCardStatus() {
    // Allocate memory for FirebaseJson objects on the heap
    FirebaseJson* jsonObjectUsersPtr = new FirebaseJson();
    FirebaseJson* jsonObjectRiotCardsPtr = new FirebaseJson();

    if (jsonObjectUsersPtr == nullptr || jsonObjectRiotCardsPtr == nullptr) {
        Serial.println("Memory allocation failed");
        // Handle the failure case accordingly
        return;
    }

    const char* path = "users/";

    // Populate jsonObjectUsers using Firebase
    Firebase.Firestore.listDocuments(&fbdo, FIREBASE_PROJECT_ID, "", path, 100, "", "", "", false);
    jsonObjectUsersPtr->setJsonData(fbdo.payload().c_str());
    //Serial.println(jsonObjectUsersPtr->toString(Serial,true));

    int i = 0;
    FirebaseJsonData jsonData;

    char fieldPath[64];
    strcpy(fieldPath, "documents/[0]/fields/userType/stringValue");
  
    while (jsonObjectUsersPtr->get(jsonData, "documents/[" + String(i) + "]/fields/userType/stringValue")) {
        if (jsonData.stringValue == "deleted") {
            const char* uID = getDataFromJsonObject(*jsonObjectUsersPtr, ("documents/[" + String(i) + "]/fields/id/stringValue").c_str()).c_str();
            *jsonObjectRiotCardsPtr = firestoreGetJson("riotCards/riot-cards");
            String index = firestoreCompare(*jsonObjectRiotCardsPtr,                
                                             "fields/riotCardList/arrayValue/values",
                                             uID,
                                             "mapValue/fields/id/stringValue",
                                             false);

            jsonObjectRiotCardsPtr->set("fields/riotCardList/arrayValue/values/[" + String(index) + "]/mapValue/fields/riotCardStatus/stringValue","inactive");
            jsonObjectRiotCardsPtr->set("fields/riotCardList/arrayValue/values/[" + String(index) + "]/mapValue/fields/inOrOut/stringValue","out");
            
            //Serial.println(jsonObjectRiotCardsPtr->toString(Serial, true));
            
            Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", "riotCards/riot-cards", jsonObjectRiotCardsPtr->raw(), "riotCardList");

                delete[] uID;
        }
                    memset(fieldPath, 0, sizeof(fieldPath));
                    strcpy(fieldPath, "documents");
                    strcat(fieldPath, "/[");
                    strcat(fieldPath, String(i).c_str());
                    strcat(fieldPath, "]/");
                    strcat(fieldPath, "fields/userType/stringValue");
        i++;
    }

    // Clean up dynamically allocated memory
    delete jsonObjectUsersPtr;
    delete jsonObjectRiotCardsPtr;
}
*/

void updateRiotCardStatus() {

}

/*
bool firestoreGetJsonD(FirebaseJson* jsonObject, const char* documentPath) {
    if (WiFi.status() == WL_CONNECTED && Firebase.ready()) {
        if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath, "")) {
            if (jsonObject == nullptr) {
                *jsonObject = (FirebaseJson*)malloc(sizeof(FirebaseJson)); // Allocate on heap only if it's null
            } else {
                (jsonObject)->clear(); // Clear the existing data from the object
            }
            jsonObject->setJsonData(fbdo.payload().c_str());
            return true; // Return true to indicate success
        }
    }
    return false; // Return false if something goes wrong or not connected
}*/




    /**
     * Resets inOrOut status of all instances in the riotCards
     *
     * @return none
     *
     */
void resetInOrOutStatus() {
    int pageSize = 1; // do not set above 1
    bool firstPage = true;
    FirebaseJson riotCardsList;
    FirebaseJsonData nextPageToken;
    FirebaseJsonData riotCardID;
    FirebaseJson cardObject;
    const char* path = "riotCards/";

    while(riotCardsList.get(nextPageToken,
    "nextPageToken"
    ) || firstPage) {
        if (!firstPage) {
        //Serial.println(nextPageToken.stringValue);
        Firebase.Firestore.listDocuments(&fbdo, FIREBASE_PROJECT_ID, "", path, pageSize, nextPageToken.stringValue, "", "riotCardID", false);
        riotCardsList.setJsonData(fbdo.payload().c_str());

        riotCardsList.get(riotCardID, "documents/[0]/fields/riotCardID/stringValue");
        char riotCardPath[64];
        sprintf(riotCardPath, "riotCards/%s", riotCardID.stringValue.c_str());
        //Serial.println(buffer);
        if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", riotCardPath, "")) {
            cardObject.setJsonData(fbdo.payload().c_str());
            cardObject.set("fields/inOrOut/stringValue", "out");
            Firebase.Firestore.patchDocument(&fbdo,
            FIREBASE_PROJECT_ID,
            "",
            riotCardPath,
            cardObject.raw(),
            "inOrOut"
            );
            
            //Serial.println(cardObject->toString(Serial, true));
        } else {
            //Serial.println(fbdo.payload());
        }
       
        } else {
        //Serial.println(nextPageToken.stringValue);
        Firebase.Firestore.listDocuments(&fbdo, FIREBASE_PROJECT_ID, "", path, pageSize, "", "", "riotCardID", false);
        riotCardsList.setJsonData(fbdo.payload().c_str());

        //Serial.println(riotCardsList.toString(Serial, true));

        firstPage = false;
        }


    }

        


/*
            char documentPath[64];
    sprintf(documentPath, "riotCards/%s", riotCardID);

    //jsonRiotCards.set("fields/inOrOut/stringValue", updateValue);
    Serial.println(firestoreGetJson(documentPath).toString(Serial, true));
    Serial.println(ESP.getFreeHeap());
    
    firestoreUpdateField(
        firestoreGetJson(documentPath),
        documentPath,
        "fields/inOrOut/stringValue",
        updateValue
    );*/

    /*
    if(Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath, jsonRiotCards.raw(), "riotCardList")){
    //Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    } else {
        Serial.println("Error Patching Document!");
    }
*/
}


class MyClass {
private:
    FirebaseData *fb;
    FirebaseJson *json;
    String documentPath;

public:
    MyClass() {
        fb = new FirebaseData();
        json = new FirebaseJson();
            
        if (fb == nullptr || json == nullptr) {
        Serial.println("Memory allocation failed");
        // Handle the failure case accordingly
        return;
        }
    
        documentPath = "users/"; // Set your document path
        //fb->setBSSLBufferSize(4096, 1024);
        //fb->setResponseSize(2048);
    }
    
    ~MyClass() {
        delete fb;
        delete json;
    }

    void test() {
        //yield();
        //Firebase.Firestore.getDocument(fb, FIREBASE_PROJECT_ID, "", documentPath.c_str());
        Firebase.Firestore.listDocuments(fb, FIREBASE_PROJECT_ID, "", "users/", 10, "", "", "", false);
        //Serial.println("a");
        json->setJsonData(fb->payload().c_str());
        Serial.println(json->toString(Serial, true));

    }
};

void testPtr() {
    MyClass *ptr = new MyClass();
    ptr->test();
    delete ptr;
}




