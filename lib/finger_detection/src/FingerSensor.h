
#ifndef DUNGNV_FINGER_SENSOR_H
#define DUNGNV_FINGER_SENSOR_H

#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <SSD1306.h>
#include <vector>
#include <string>
#include "SPIFFS.h"

// #include <SoftwareSerial.h>
// #define ESP8266
HardwareSerial mySerial(2); // => RX => D16, TX => D17
// SoftwareSerial mySerial(16, 17, false, 256);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

std::vector<String> finger_username;

uint8_t id;

const int led = 2; // GPIO5

enum FINGER_MODE
{
    FINGER_CREATION = 1,
    FINGER_DETECTION = 2
};

static int finger_mode = FINGER_DETECTION;

uint8_t readnumber(void)
{
    uint8_t num = 0;

    while (num == 0)
    {
        while (!Serial.available())
            ;
        num = Serial.parseInt();
    }
    return num;
}

void finger_write_to_spiffs()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("Can not start SPIFFS");
        return;
    }

    // Ghi dữ liệu vào tệp tin
    File file_username = SPIFFS.open("/finger_username.txt", "w");
    if (!file_username)
    {
        Serial.println("Can not open file to write");
        return;
    }

    // Lưu từng chuỗi trong vector vào tệp tin
    for (const auto &str : finger_username)
    {
        file_username.println(str);
    }

    file_username.close();
}

void finger_read_from_spiffs()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("Can not start SPIFFS");
        return;
    }

    // Đọc dữ liệu từ tệp tin
    File file_username = SPIFFS.open("/finger_username.txt", "r");
    if (!file_username)
    {
        Serial.println("Can not open file to read");
        return;
    }

    finger_username.clear();

    // Đọc từng chuỗi từ tệp và thêm vào vector
    while (file_username.available())
    {
        finger_username.push_back(file_username.readStringUntil('\n'));
    }

    file_username.close();

    Serial.println("Completed to read finger_username");
    // Kiểm tra dữ liệu đã đọc
    for (const auto &str : finger_username)
    {
        Serial.println(str);
    }
}

uint8_t getFingerprintEnroll()
{
    finger.getTemplateCount();
    id = finger.templateCount + 1;
    int p = -1;
    Serial.print("Waiting for valid finger to enroll as #");
    ssd1306_clear_and_write_to_row(10, "Waiting for valid finger to enroll as #" + String(id));
    Serial.println(id);
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            ssd1306_write_to_row(20, "Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.println(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            ssd1306_write_to_row(20, "Image taken");
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            ssd1306_write_to_row(20, "Imaging error");
            break;
        default:
            Serial.println("Unknown error");
            ssd1306_write_to_row(20, "Unknown error");
            break;
        }
    }

    // OK success!
    p = finger.image2Tz(1);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        ssd1306_write_to_row(30, "Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        ssd1306_write_to_row(30, "Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        ssd1306_write_to_row(30, "Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        ssd1306_write_to_row(30, "Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        ssd1306_write_to_row(30, "Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        ssd1306_write_to_row(30, "Unknown error");
        return p;
    }

    Serial.println("Remove finger");
    ssd1306_write_to_row(40, "Remove finger");
    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER)
    {
        p = finger.getImage();
    }
    Serial.print("ID ");
    Serial.println(id);
    p = -1;
    Serial.println("Place same finger again");
    ssd1306_write_to_row(50, "Place same finger again");
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.print(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            break;
        default:
            Serial.println("Unknown error");
            break;
        }
    }

    // OK success!
    p = finger.image2Tz(2);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    // OK converted!
    Serial.print("Creating model for #");
    ssd1306_clear_and_write_to_row(10, "Creating model for #" + String(id));

    finger_username.push_back(String(id));
    finger_write_to_spiffs();

    Serial.println(id);
    Serial.println("Change to detection mode!");
    ssd1306_clear_and_write_to_row(10, "Change to detection mode!");
    finger_mode = FINGER_DETECTION;

    p = finger.createModel();
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Prints matched!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_ENROLLMISMATCH)
    {
        Serial.println("Fingerprints did not match");
        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }

    Serial.print("ID ");
    Serial.println(id);
    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Stored!");
        finger_mode = FINGER_DETECTION;
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
        Serial.println("Could not store in that location");
        return p;
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
        Serial.println("Error writing to flash");
        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }
}

uint8_t getFingerprintEnroll(String username, int index)
{
    // if (index != -1)
    // {
    //     id = index;
    // }
    // else
    // {
    //     finger.getTemplateCount();
    //     id = finger.templateCount + 1;
    // }

    finger.getTemplateCount();
    id = finger.templateCount + 1;

    int p = -1;
    Serial.print("Waiting for valid finger to enroll as #" + String(id) + "for username: " + username);

    ssd1306_clear_and_write_to_row(10, "Waiting for valid finger to enroll as #" + String(id) + "for username: " + username);

    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            ssd1306_write_to_row(20, "Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.println(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            ssd1306_write_to_row(20, "Image taken");
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            ssd1306_write_to_row(20, "Imaging error");
            break;
        default:
            Serial.println("Unknown error");
            ssd1306_write_to_row(20, "Unknown error");
            break;
        }
    }

    // OK success!
    p = finger.image2Tz(1);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        ssd1306_write_to_row(30, "Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        ssd1306_write_to_row(30, "Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        ssd1306_write_to_row(30, "Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        ssd1306_write_to_row(30, "Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        ssd1306_write_to_row(30, "Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        ssd1306_write_to_row(30, "Unknown error");
        return p;
    }

    Serial.println("Remove finger");
    ssd1306_write_to_row(40, "Remove finger");
    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER)
    {
        p = finger.getImage();
    }
    Serial.print("ID ");
    Serial.println(id);
    p = -1;
    Serial.println("Place same finger again");
    ssd1306_write_to_row(50, "Place same finger again");
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.print(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            break;
        default:
            Serial.println("Unknown error");
            break;
        }
    }

    // OK success!
    p = finger.image2Tz(2);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    // OK converted!
    Serial.print("Creating model for #");
    ssd1306_clear_and_write_to_row(10, "Creating model for #" + String(id));

    finger_username.push_back(username);
    finger_write_to_spiffs();

    Serial.println(id);
    Serial.println("Change to detection mode!");
    ssd1306_clear_and_write_to_row(10, "Change to detection mode!");
    finger_mode = FINGER_DETECTION;

    p = finger.createModel();
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Prints matched!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_ENROLLMISMATCH)
    {
        Serial.println("Fingerprints did not match");
        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }

    Serial.print("ID ");
    Serial.println(id);
    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Stored!");
        finger_mode = FINGER_DETECTION;
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
        Serial.println("Could not store in that location");
        return p;
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
        Serial.println("Error writing to flash");
        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }
}

uint8_t getFingerprintID()
{
    uint8_t p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
    case FINGERPRINT_NOFINGER:
        Serial.println("No finger detected");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    // OK success!

    p = finger.image2Tz();
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    // OK converted!
    p = finger.fingerFastSearch();
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Found a print match!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_NOTFOUND)
    {
        Serial.println("Did not find a match");
        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }

    // found a match!
    Serial.print("Found ID #");
    Serial.print(finger.fingerID);
    Serial.print(" with confidence of ");
    Serial.println(finger.confidence);
    return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez()
{
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK)
        return -1;

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK)
        return -1;

    p = finger.fingerFastSearch();
    if (p != FINGERPRINT_OK)
        return -1;

    // found a match!
    Serial.print("Found ID #");
    Serial.print(finger.fingerID);
    Serial.print(" with confidence of ");
    Serial.println(finger.confidence);

    if (finger.fingerID == 1 && finger.confidence > 100)
    {
        digitalWrite(led, HIGH);
    }
    if (finger.fingerID == 2 && finger.confidence > 100)
    {
        digitalWrite(led, LOW);
    }

    Serial.println("Detected user: " + String(finger.fingerID));
    // ssd1306_header_and_write_to_row(FINGER_HEADER_TEXT, 10, "Detected user: " + finger_username.at(finger.fingerID - 1));
    ssd1306_header_and_write_to_row(FINGER_HEADER_TEXT, 10, "Detected user: " + String(finger.fingerID));

    return finger.fingerID;
}

uint8_t deleteFingerprint(uint8_t id)
{
    Serial.print("Deleting ID #");
    Serial.println(id);

    uint8_t p = -1;

    p = finger.deleteModel(id);

    if (p == FINGERPRINT_OK)
    {
        Serial.println("Deleted!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
        Serial.println("Could not delete in that location");
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
        Serial.println("Error writing to flash");
    }
    else
    {
        Serial.print("Unknown error: 0x");
        Serial.println(p, HEX);
    }

    return p;
}

void empty_finger_database()
{
    Serial.println("Delete All models in finger database!");
    finger.emptyDatabase();
    finger_username.clear();
    finger_write_to_spiffs();
    ssd1306_write_to_row(20, "Now finger database is empty :)");
    Serial.println("Now database is empty :)");
}

void init_finger_sensor()
{
    pinMode(led, OUTPUT);
    Serial.println("\n\nAdafruit finger detect test");

    // set the data rate for the sensor serial port
    finger.begin(57600);

    if (finger.verifyPassword())
    {
        Serial.println("Found fingerprint sensor!");
    }
    else
    {
        Serial.println("Did not find fingerprint sensor :(");
        // while (1)
        // {
        //     delay(1);
        // }
    }

    // empty_finger_database();
    finger_read_from_spiffs();
    finger.getTemplateCount();
    Serial.print("Sensor contains ");
    Serial.print(finger.templateCount);
    Serial.println(" templates");
    Serial.println("Waiting for valid finger...");
}
#endif