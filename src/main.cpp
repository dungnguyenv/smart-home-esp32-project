/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/
#include <FingerSensor.h>
#include <SSD1306.h>
#include <SPI.h>
#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <RFID.h>
#include <ArduinoJson.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Tang 2"
#define WIFI_PASSWORD "mattrinho"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyDwwFGgDv-IplfIzjzxnnaGX77pr0_Hpag"

/* 3. Define the RTDB URL */
#define DATABASE_URL "smart-home-db-8db49-default-rtdb.asia-southeast1.firebasedatabase.app" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "esp32.01@smarthome.com"
#define USER_PASSWORD "password"

// Define Firebase Data object
FirebaseData stream;
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

#define FINGER_TOUCH_PIN 4

String parentPath = "/smart-home/living-room/sensors/open_door_event";
String childPath[2] = {"/rfid", "/finger"};

void create_new_finger();
void handleFingerButtonTouching();
void firebase_init();
void open_liv_door();

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  delay(100);

  rfid_init();

  pinMode(FINGER_TOUCH_PIN, INPUT);

  init_finger_sensor();
  ssd1306_init();
  ssd1306_clear();
  ssd1306_write_to_row(10, "Hello World!");

  firebase_init();
  // finger_mode = FINGER_CREATION;
}

void loop() // run over and over again
{
  if (finger_mode == FINGER_DETECTION)
  {
    int status = getFingerprintIDez();
    if (status > 0)
    {
      open_liv_door();
    }

    delay(50); // don't ned to run this at full speed.
    if (!digitalRead(FINGER_TOUCH_PIN))
    {
      handleFingerButtonTouching();
    }
  }
  else
  {
    create_new_finger();
  }

  int status = rfid_handle();
  if (status > 0)
  {
    open_liv_door();
  }

  delay(10);
}

void handleFingerButtonTouching()
{
  if (!digitalRead(FINGER_TOUCH_PIN))
  {
    Serial.println("Finger Button TOUCHING: ...");
    ssd1306_header_and_write_to_row(FINGER_HEADER_TEXT, 10, "Finger Button TOUCHING: ...");
    long startPressTime = millis();
    while (!digitalRead(FINGER_TOUCH_PIN))
    {
      delay(1000);
      if (millis() - startPressTime > 10000)
      {
        Serial.println("Clear all Fingers");
        ssd1306_header_and_write_to_row(FINGER_HEADER_TEXT, 10, "CLEAR ALL FINGER");
        break;
      }
      else if (millis() - startPressTime > 5000)
      {
        Serial.println("Change to Finger Creation Mode!");
        ssd1306_header_and_write_to_row(FINGER_HEADER_TEXT, 10, "FINGER CREATION MODE");
        // finger_mode = FINGER_CREATION;
      }
    }
    Serial.println("TOUCHED: " + String(millis() - startPressTime));

    if (millis() - startPressTime > 10000)
    {
      Serial.println("Clear all Fingers");
      ssd1306_header_and_write_to_row(FINGER_HEADER_TEXT, 10, "CLEAR ALL FINGER");
      empty_finger_database();
    }
    else if (millis() - startPressTime > 5000)
    {
      Serial.println("Change to Finger Creation Mode!");
      ssd1306_header_and_write_to_row(FINGER_HEADER_TEXT, 10, "FINGER CREATION MODE");
      finger_mode = FINGER_CREATION;
    }
  }
}

void create_new_finger()
{
  Serial.println("Ready to enroll a fingerprint!");
  while (!getFingerprintEnroll())
  {
    if (finger_mode == FINGER_DETECTION)
    {
      finger.getTemplateCount();
      Serial.print("Sensor contains ");
      Serial.print(finger.templateCount);
      Serial.println(" templates");
      Serial.println("Waiting for valid finger...");
      break;
    }
  }

  if (finger_mode == FINGER_DETECTION)
  {
    finger.getTemplateCount();
    Serial.print("Sensor contains ");
    Serial.print(finger.templateCount);
    Serial.println(" templates");
    Serial.println("Waiting for valid finger...");
  }
}

void streamCallback(StreamData data)
{
  Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); // see addons/RTDBHelper.h
  Serial.println();
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());

  Firebase.getJSON(fbdo, parentPath);
  FirebaseJson *json = fbdo.to<FirebaseJson *>();

  DynamicJsonDocument doc(1024);

  // Deserialize the FirebaseJson object into the DynamicJsonDocument
  DeserializationError error = deserializeJson(doc, json->raw());

  // Check if the deserialization was successful
  if (error)
  {
    Serial.print("Error during deserialization: ");
    Serial.println(error.c_str());
    return;
  }
  // Now you can work with the JSON data using the DynamicJsonDocument
  // For example, to access the "add_new" object:

  JsonObject rfid_add_new = doc["rfid"]["add_new"].as<JsonObject>();

  // Access the values inside the "add_new" object
  if (rfid_add_new.containsKey("trigger") && rfid_add_new.containsKey("username"))
  {
    String triggerValue = rfid_add_new["trigger"].as<String>();
    String usernameValue = rfid_add_new["username"].as<String>();
    // Now you can work with the values
    Serial.println("add_new -> trigger: " + triggerValue);
    Serial.println("add_new -> username: " + usernameValue);
    if (triggerValue == "1")
    {
      Serial.println("Add new card!");
      rfid_add_new_card(usernameValue);
      Serial.printf("Update trigger value to 0... %s\n", Firebase.setInt(fbdo, (parentPath + "/rfid/add_new/trigger"), 0) ? "ok" : fbdo.errorReason().c_str());
    }
  }
  else
  {
    Serial.println("Missing 'trigger' or 'username' keys inside 'add_new' object.");
  }

  JsonObject finger_add_new = doc["finger"]["add_new"].as<JsonObject>();
  // Access the values inside the "add_new" object
  if (finger_add_new.containsKey("trigger") && finger_add_new.containsKey("username"))
  {
    String triggerValue = finger_add_new["trigger"].as<String>();
    String usernameValue = finger_add_new["username"].as<String>();
    int index = finger_add_new["trigger"].as<int>();

    // Now you can work with the values
    Serial.println("add_new -> trigger: " + triggerValue);
    Serial.println("add_new -> username: " + usernameValue);
    Serial.println("add_new -> index: " + String(index));
    if (triggerValue == "1")
    {
      Serial.println("Add new Finger!");
      getFingerprintEnroll(usernameValue, -1);
      Serial.printf("Update trigger value to 0... %s\n", Firebase.setInt(fbdo, (parentPath + "/finger/add_new/trigger"), 0) ? "ok" : fbdo.errorReason().c_str());
    }
  }
  else
  {
    Serial.println("Missing 'trigger' or 'username' keys inside 'add_new' object.");
  }
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void firebase_init()
{
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

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

  if (!Firebase.beginStream(stream, parentPath))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.setStreamCallback(stream, streamCallback, streamTimeoutCallback);
}

void open_liv_door()
{
  FirebaseJson json;

  json.set("status", 1);
  // now we will set the timestamp value at Ts
  json.set("time/.sv", "timestamp"); // .sv is the required place holder for sever value which currently supports only string "timestamp" as a value
  Serial.printf("Open door ... %s\n", Firebase.setJSON(fbdo, ("/smart-home/living-room/devices/liv-door"), json) ? "ok" : fbdo.errorReason().c_str());
}