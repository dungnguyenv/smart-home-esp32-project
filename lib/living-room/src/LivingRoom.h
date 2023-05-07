#ifndef LIVING_ROOM_H
#define LIVING_ROOM_H

#include <Arduino.h>
#include <FirebaseESP32.h>

class LivingRoom
{
private:
    FIREBASE_CLASS *Firebase;
    FirebaseData *stream;

    static String parentPath;
    static String childPath[4];
    static String logsPath;
    static LivingRoom LivingRoomObject;

    static void streamCallback(MultiPathStreamData stream);
    static void streamTimeoutCallback(bool timeout);

    LivingRoom();
    void handleDoorEvent();
    void handleLight1Event();
    void handleLight2Event();
    void handleTvEvent();

public:
    ~LivingRoom();

    void init(FIREBASE_CLASS *Firebase, FirebaseData *stream);
    void updateTemperatureAndHuminity();
    static LivingRoom getLivingRoomObject();
    static String concat2String(String s1, String s2, String s3 = "")
    {
        s1.concat(s2);
        s1.concat(s3);
        return s1;
    }
    String currentTimestamp()
    {
        FirebaseData fbdo;
        Firebase->setTimestamp(fbdo, "/timestamp");
        return String(fbdo.to<uint64_t>());
    }
};

#endif