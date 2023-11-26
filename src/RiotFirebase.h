#ifndef RIOTFIREBASE_H_
#define RIOTFIREBASE_H_

#include <Firebase_ESP_Client.h>

extern FirebaseData fbdo;
extern FirebaseAuth auth;
extern FirebaseConfig config;

extern const u_int8_t FIREBASE_PIN;

/// @brief

int initFirebase();
inline bool firestoreGetJson(FirebaseJson* jsonObject, const char* documentPath);
inline bool firestoreUpdateField(FirebaseJson* jsonObject, const char* documentPath, const char* updateWhere, const char* updateValue);
String getDataFromJsonObject(FirebaseJson* jsonObject, const char* fieldPath);
void updateRiotCardStatus();
void resetInOrOutStatus();
bool updateNumberOfPeople();
String firebaseJsonIterator(FirebaseJson* jsonObject, String pathToArray, const char* updateField,const char* funcValue, void (*func) (const char*, const char*, int*));
bool uploadAllFirestoreTasks(FirebaseJson* jsonObjectRiotCard, const char* riotCardID);

#endif