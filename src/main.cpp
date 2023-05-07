/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP32
 *
 * Copyright (c) 2023 mobizt
 *
 */

// This example shows how to set and push timestamp (server time) which is the server variable that suopported by Firebase

#include <Arduino.h>
#include <FirebaseCommons.h>

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

void setup()
{

  Serial.begin(115200);
  Serial.println();
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  firebaseConfig(config, auth);

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);
}

void loop()
{
  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if (Firebase.ready() && !taskCompleted)
  {
    taskCompleted = true;

    Serial.printf("Set timestamp... %s\n", Firebase.setTimestamp(fbdo, "/test/timestamp") ? "ok" : fbdo.errorReason().c_str());

    if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
    {
      // In setTimestampAsync, the following timestamp will be 0 because the response payload was ignored for all async functions.

      // Timestamp saved in millisecond, get its seconds from int value
      Serial.print("TIMESTAMP (Seconds): ");
      Serial.println(fbdo.to<int>());

      // Or print the total milliseconds from double value
      // Due to bugs in Serial.print in Arduino library, use printf to print double instead.
      printf("TIMESTAMP (milliSeconds): %lld\n", fbdo.to<uint64_t>());
    }

    Serial.printf("Get timestamp... %s\n", Firebase.getDouble(fbdo, "/test/timestamp") ? "ok" : fbdo.errorReason().c_str());
    if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
      printf("TIMESTAMP: %lld\n", fbdo.to<uint64_t>());

    // To set and push data with timestamp, requires the JSON data with .sv placeholder
    FirebaseJson json;

    json.set("Data", "Hello");
    // now we will set the timestamp value at Ts
    json.set("Ts/.sv", "timestamp"); // .sv is the required place holder for sever value which currently supports only string "timestamp" as a value

    // Set data with timestamp
    Serial.printf("Set data with timestamp... %s\n", Firebase.setJSON(fbdo, "/test/set/data", json) ? fbdo.to<FirebaseJson>().raw() : fbdo.errorReason().c_str());

    // Push data with timestamp
    Serial.printf("Push data with timestamp... %s\n", Firebase.pushJSON(fbdo, "/test/push/data", json) ? "ok" : fbdo.errorReason().c_str());

    // Get previous pushed data
    // Serial.printf("Get previous pushed data... %s\n", Firebase.getJSON(fbdo, "/test/push/data/" + fbdo.pushName()) ? fbdo.to<FirebaseJson>().raw() : fbdo.errorReason().c_str());
  }
}