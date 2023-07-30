
#ifndef DUNGNV_RFID_H
#define DUNGNV_RFID_H

#include <SPI.h>
#include <MFRC522.h>
#include <vector>
#include <array>
#include "SPIFFS.h"

// #include <ESP32Servo.h>

#define SS_PIN 5
#define RST_PIN 2
#define DOOR_PIN 27
#define RELAY_PIN 26

// Servo doorServo;
MFRC522 rfid(SS_PIN, RST_PIN);

std::vector<String> rfid_username;
// = {
//     "dungnguyen",
//     "andleduc", "demo"};
std::vector<std::array<uint8_t, 4>> rfid_authorize_uid;
// = {
//     {0xEC, 0x94, 0x4A, 0x2E},
//     {0xFA, 0xB4, 0x2B, 0xB3},
//     {0x54, 0xBD, 0xF7, 0x51}};

boolean openDoor = false;
boolean curtainsUp = false;

float LIGHT_THRESHOLD = 1000.0;
float lightSensorValue;

unsigned long openDoorDuration = 2000;

void handleOpenDoor();
int checkRfidMatchedIndex(MFRC522::Uid uid);
void rfid_write_to_spiffs();
void rfid_read_from_spiffs();

void rfid_init()
{
    SPI.begin();                  // init SPI bus
    rfid.PCD_Init();              // init MFRC522
    pinMode(RELAY_PIN, OUTPUT);   // initialize pin as an output.
    digitalWrite(RELAY_PIN, LOW); // deactivate the relay

    // rfid_write_to_spiffs();
    rfid_read_from_spiffs();

    Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
}

int rfid_handle()
{
    unsigned long beginOpenDoorTime;
    if (rfid.PICC_IsNewCardPresent())
    { // new tag is available
        if (rfid.PICC_ReadCardSerial())
        { // NUID has been readed
            MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

            int _detected_uid_index = checkRfidMatchedIndex(rfid.uid);
            if (_detected_uid_index != -1)
            {
                Serial.println("Username detected: " + rfid_username[_detected_uid_index]);
                ssd1306_header_and_write_to_row(RFID_HEADER_TEXT, 10, "Detected User: " + rfid_username[_detected_uid_index]);
                return 1;
            }
            else
            {
                Serial.print("Unauthorized Tag with UID:");
                for (int i = 0; i < rfid.uid.size; i++)
                {
                    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
                    Serial.print(rfid.uid.uidByte[i], HEX);
                }
                Serial.println();
            }

            rfid.PICC_HaltA();      // halt PICC
            rfid.PCD_StopCrypto1(); // stop encryption on PCD
        }
    }
    openDoor = false;

    return 0;
}

void rfid_add_new_card(String username)
{
    Serial.println("Please tap an RFID/NFC tag on the RFID-RC522 reader");
    ssd1306_header_and_write_to_row(RFID_HEADER_TEXT, 10, "Please tap an RFID/NFC tag:");
    while (!rfid.PICC_IsNewCardPresent())
        ;
    if (rfid.PICC_ReadCardSerial())
    { // NUID has been readed
        MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

        int _detected_uid_index = checkRfidMatchedIndex(rfid.uid);
        if (_detected_uid_index != -1)
        {
            Serial.println("Error: card id is available for: " + rfid_username[_detected_uid_index]);
            Serial.println("Please tap another RFID/NFC card!");
            ssd1306_write_to_row(30, "Error: card id is available for: " + rfid_username[_detected_uid_index]);
            ssd1306_write_to_row(50, "Please tap another RFID/NFC card!");
        }
        else
        {
            Serial.print("Tag with UID:");
            ssd1306_write_to_row(20, "Tag with UID for username: " + username);
            std::array<uint8_t, 4> uid;
            for (int i = 0; i < rfid.uid.size; i++)
            {
                uid[i] = rfid.uid.uidByte[i];
                Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
                Serial.print(rfid.uid.uidByte[i], HEX);
            }

            rfid_username.push_back(username);
            rfid_authorize_uid.push_back(uid);

            rfid_write_to_spiffs();

            Serial.println("Registered new user card successfully!");
            ssd1306_write_to_row(30, "Registered new user card successfully!");
            Serial.println();
        }

        rfid.PICC_HaltA();      // halt PICC
        rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
}

boolean checkRFID(byte &rfidValue)
{
    // TODO: viết hàm check byte -> đỡ lặp code
    return true;
}

int checkRfidMatchedIndex(MFRC522::Uid uid)
{
    for (int i = 0; i < rfid_authorize_uid.size(); i++)
    {
        if (uid.uidByte[0] == rfid_authorize_uid[i][0] &&
            uid.uidByte[1] == rfid_authorize_uid[i][1] &&
            uid.uidByte[2] == rfid_authorize_uid[i][2] &&
            uid.uidByte[3] == rfid_authorize_uid[i][3])
        {
            return i;
        }
    }
    return -1;
}

void rfid_write_to_spiffs()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("Can not start SPIFFS");
        return;
    }

    // Ghi dữ liệu vào tệp tin
    File file = SPIFFS.open("/rfid_uid.txt", "w");
    if (!file)
    {
        Serial.println("Can not open file to write");
        return;
    }

    // Lưu mỗi mảng trong vector vào tệp tin
    for (const auto &arr : rfid_authorize_uid)
    {
        file.write(reinterpret_cast<const uint8_t *>(arr.data()), sizeof(arr));
    }

    file.close();

    // Ghi dữ liệu vào tệp tin
    File file_username = SPIFFS.open("/rfid_username.txt", "w");
    if (!file_username)
    {
        Serial.println("Can not open file to write");
        return;
    }

    // Lưu từng chuỗi trong vector vào tệp tin
    for (const auto &str : rfid_username)
    {
        file_username.println(str);
    }

    file_username.close();
}

void rfid_read_from_spiffs()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("Can not start SPIFFS");
        return;
    }

    // Đọc dữ liệu từ tệp tin
    File file = SPIFFS.open("/rfid_uid.txt", "r");
    if (!file)
    {
        Serial.println("Can not open file to read");
        return;
    }

    rfid_authorize_uid.clear();

    // Đọc từng mảng từ tệp và thêm vào vector
    std::array<uint8_t, 4> buffer;
    while (file.read(reinterpret_cast<uint8_t *>(buffer.data()), sizeof(buffer)))
    {
        rfid_authorize_uid.push_back(buffer);
    }

    file.close();
    Serial.println("Completed to read rfid_authorize_uid");
    // Kiểm tra dữ liệu đã đọc
    for (const auto &arr : rfid_authorize_uid)
    {
        for (const auto &value : arr)
        {
            Serial.print(value, HEX);
            Serial.print(" ");
        }
        Serial.println();
    }

    // Đọc dữ liệu từ tệp tin
    File file_username = SPIFFS.open("/rfid_username.txt", "r");
    if (!file_username)
    {
        Serial.println("Can not open file to read");
        return;
    }

    rfid_username.clear();

    // Đọc từng chuỗi từ tệp và thêm vào vector
    while (file_username.available())
    {
        rfid_username.push_back(file_username.readStringUntil('\n'));
    }

    file_username.close();

    Serial.println("Completed to read rfid_username");
    // Kiểm tra dữ liệu đã đọc
    for (const auto &str : rfid_username)
    {
        Serial.println(str);
    }
}

#endif
