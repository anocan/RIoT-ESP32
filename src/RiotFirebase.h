#ifndef RIOTFIREBASE_H_
#define RIOTFIREBASE_H_

#include "credentials.h"
#include <Firebase_ESP_Client.h>

class RIoTFirebase {
private:
  FirebaseData fbdo;
  FirebaseAuth auth;
  FirebaseConfig config;

  static void compareAndCount(const char *value1, const char *value2,
                              int *count);
  static void compareAndReturn(const char *value1, const char *value2,
                               int *count);
  static void setAllInOrOutToOut(const char *riotCardID,
                                 const char *updateValue, int *count);
  static void compareAndUpdateRiotCard(const char *riotCardID,
                                       const char *comparisonValue, int *count);
  String getNoOfPeople();

public:
  RIoTFirebase() {
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    fbdo.setBSSLBufferSize(4096, 1024);
    fbdo.setResponseSize(2048);
  }
  /**
   *
   * @brief Initialize the Firebase and configure debug and RIoT system
   * settings.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note BSSLBufferSize and ResponseSize can be increased to have larger
   * file operations.
   */
  bool initFirebase();

  /**
   *
   * @brief Get FirebaseJson object from Firebase Firestore for given document
   * path.
   * @param jsonObject Json object to be written into.
   * @param documentPath The document path to the Firestore, e.g.
   * "riotCards/{121a32ff}".
   * @return Boolean value, indicates the success of the operation.
   *
   */
  bool firestoreGetJson(FirebaseJson *jsonObject, const char *documentPath);

  /**
   *
   * @brief Update FirebaseFirestore using FirebaseJson.
   * @param jsonObject FirebaseJson object that will update the Firebase
   * Firestore.
   * @param documentPath The document path to the Firestore, e.g.
   * "riotCards/{121a32ff}".
   * @param updateField The path to the update field, e.g.
   * "fields/riotCards/{121a32ff}/riotCardID/stringValue".
   * @param updateValue The field value to update.
   * @return Boolean value, indicates the success of the operation.
   *
   * @note For the param. updateWhere value should be specified, such as
   * stringValue, intValue... corresponding to the database data type.
   */
  inline bool firestoreUpdateField(FirebaseJson *jsonObject,
                                   const char *documentPath,
                                   const char *updateField,
                                   const char *updateValue);
  /**
   *
   * @brief Get data from FirebaseJson object for a given fieldPath.
   * @param jsonObject FirebaseJson object to retrieve data.
   * @param fieldPath Path to the Firestore element to be returned.
   * @return FirebaseJsonData as String.
   *
   */
  String getDataFromJsonObject(FirebaseJson *jsonObject, const char *fieldPath);

  /**
   * @brief Updates RIoT card status when user account is deleted.
   * @return None.
   *
   */
  void updateRiotCardStatus();

  /**
   * @brief Resets inOrOut status of all instances in the riotCards.
   * @return None.
   *
   */
  void resetInOrOutStatus();

  /**
   * @brief Updates the number of people in the lab.
   * @return Boolean value, indicates the success of the operation.
   *
   */
  bool updateNumberOfPeople();

  /**
   *
   * @brief Iterates through the given JsonObject and works on the data with
   * given function.
   * @param jsonObject FirebaseJson object to be iterated.
   * @param pathToArray Path to the array element inside the FirebaseJson
   * object, e.g. "documents/".
   * @param updateField The relative path to the update field, e.g.
   * "riotCardID".
   * @param funcValue An outside value to be passed into the function, e.g.
   * "deleted".
   * @param func The function to handle the incoming data, e.g.
   * "&compareAndUpdateRiotCard".
   * @return FirebaseJsonData as String or "NULL" if nothing returns.
   *
   * @note Before using it is highly suggested to review the function
   * definiton and other sub-functions such as "compareAndUpdateRiotCard()".
   */
  String firebaseJsonIterator(FirebaseJson *jsonObject, String pathToArray,
                              const char *updateField, const char *funcValue,
                              void (*func)(const char *, const char *, int *));

  /**
   * Upload and complete all FirebaseFirestore related tasks
   *
   * @param jsonObjectRiotCard RIoT card's json object.
   * @param riotCardID RIoT card id.
   * @return Boolean value, indicates the success of the operation.
   *
   */
  bool uploadAllFirestoreTasks(FirebaseJson *jsonObjectRiotCard,
                               const char *riotCardID);
};

#endif