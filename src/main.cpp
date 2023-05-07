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

#include <Arduino.h>
#include <FirebaseCommons.h>
#include <LivingRoom.h>

// Define Firebase Data object
FirebaseData fbdo;
FirebaseData livingRoomStream;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

LivingRoom LivingRoomObject = LivingRoom::getLivingRoomObject();

void setup()
{

  Serial.begin(115200);

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

  // The data under the node being stream (parent path) should keep small
  // Large stream payload leads to the parsing error due to memory allocation.

  // The MultiPathStream works as normal stream with the payload parsing function.

  LivingRoomObject.init(&Firebase, &livingRoomStream);
}

void loop()
{

  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();

    Serial.print("\nSet json...");

    // FirebaseJson json;

    LivingRoomObject.updateTemperatureAndHuminity();

    // for (size_t i = 0; i < 10; i++)
    // {
    //   json.set("node1/data", "v1");
    //   json.set("node1/num", count);
    //   json.set("node2/data", "v2");
    //   json.set("node2/num", count * 3);
    //   // The response is ignored in this async function, it may return true as long as the connection is established.
    //   // The purpose for this async function is to set, push and update data instantly.
    //   Firebase.setJSONAsync(fbdo, parentPath, json);
    //   count++;
    // }

    Serial.println("ok\n");
  }
}