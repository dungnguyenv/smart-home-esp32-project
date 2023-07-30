#ifndef DUNGNV_SSD1306_H
#define DUNGNV_SSD1306_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const String RFID_HEADER_TEXT = "RFID SENSOR";
const String FINGER_HEADER_TEXT = "FINGER SENSOR";
String HEADER_TEXT = FINGER_HEADER_TEXT;

void ssd1306_init()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    { // Address 0x3D for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        // for (;;)
        //     ;
    }
    delay(2000);
}

void ssd1306_write_to_row(int row, String text)
{
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, row);
    // Display static text
    display.println(text);
    display.display();
}

void ssd1306_header_text(String text)
{
    HEADER_TEXT = text;
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    // Display static text
    display.println(HEADER_TEXT);
    display.display();
}

void ssd1306_header_text()
{
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    // Display static text
    display.println(HEADER_TEXT);
    display.display();
}

void ssd1306_clear()
{
    display.clearDisplay();
    ssd1306_header_text();
}

void ssd1306_clear_and_write_to_row(int row, String text)
{
    display.clearDisplay();
    ssd1306_header_text();

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, row);
    // Display static text
    display.println(text);
    display.display();
}

void ssd1306_header_and_write_to_row(String header, int row, String text)
{
    display.clearDisplay();
    ssd1306_header_text(header);

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, row);
    // Display static text
    display.println(text);
    display.display();
}

#endif