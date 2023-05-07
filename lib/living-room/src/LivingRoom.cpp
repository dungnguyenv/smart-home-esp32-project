#include "LivingRoom.h"

String LivingRoom::logsPath = "/smart-home/living-room/logs";
String LivingRoom::parentPath = "/smart-home/living-room/devices";
String LivingRoom::childPath[4] = {"/door", "/tv", "/light1", "/light2"};
LivingRoom LivingRoom::LivingRoomObject = LivingRoom();

enum Devices
{
    DOOR = 0,
    TV = 1,
    LIGHT1 = 2,
    LIGHT2 = 3
};

void LivingRoom::init(FIREBASE_CLASS *Firebase, FirebaseData *stream)
{
    this->Firebase = Firebase;
    this->stream = stream;

    if (!Firebase->beginMultiPathStream(*stream, parentPath))
        Serial.printf("sream begin error, %s\n\n", stream->errorReason().c_str());
    Firebase->setMultiPathStreamCallback(*stream, streamCallback, streamTimeoutCallback);
}

void LivingRoom::streamCallback(MultiPathStreamData stream)
{
    size_t numChild = sizeof(childPath) / sizeof(childPath[0]);

    for (size_t i = 0; i < numChild; i++)
    {
        if (stream.get(childPath[i]))
        {
            Serial.printf("path: %s, event: %s, type: %s, value: %s%s", stream.dataPath.c_str(), stream.eventType.c_str(), stream.type.c_str(), stream.value.c_str(), i < numChild - 1 ? "\n" : "");

            switch (i)
            {
            case DOOR:
                LivingRoomObject.handleDoorEvent();
                break;
            case TV:
                LivingRoomObject.handleTvEvent();
                break;
            case LIGHT1:
                LivingRoomObject.handleLight1Event();
                break;
            case LIGHT2:
                LivingRoomObject.handleLight2Event();
                break;
            default:
                break;
            }
        }
    }

    Serial.println();

    // This is the size of stream payload received (current and max value)
    // Max payload size is the payload size under the stream path since the stream connected
    // and read once and will not update until stream reconnection takes place.
    // This max value will be zero as no payload received in case of ESP8266 which
    // BearSSL reserved Rx buffer size is less than the actual stream payload.
    Serial.printf("Received stream payload size: %d (Max. %d)\n\n", stream.payloadLength(), stream.maxPayloadLength());
}

void LivingRoom::handleDoorEvent()
{
    Serial.println("Handle Door Event");
}

void LivingRoom::handleLight1Event()
{
    Serial.println("Handle Light1 Event");
}

void LivingRoom::handleLight2Event()
{
    Serial.println("Handle Light2 Event");
}

void LivingRoom::handleTvEvent()
{
    Serial.println("Handle TV Event");
}

void LivingRoom::updateTemperatureAndHuminity()
{
    Serial.println("Update Temp and Humi");

    static float humidity = 50;
    static float temperature = 20;
    FirebaseData fbdo;
    FirebaseJson json;

    json.set("/humidity/value", humidity++);
    json.set("/humidity/time/.sv", "timestamp");
    json.set("/temperature/value", temperature++);
    json.set("/temperature/time/.sv", "timestamp");
    Firebase->setJSONAsync(fbdo, LivingRoom::concat2String(parentPath, "/dht11"), json);

    String timestamp = LivingRoomObject.currentTimestamp();
    Serial.println(timestamp);
    FirebaseJson jsonTempLog;
    jsonTempLog.set("/value", temperature++);
    jsonTempLog.set("/time/.sv", "timestamp");
    Firebase->setJSONAsync(fbdo, LivingRoom::concat2String(logsPath, "/dht11/temperature/", timestamp), jsonTempLog);
}

void LivingRoom::streamTimeoutCallback(bool timeout)
{
    if (timeout)
        Serial.println("stream timed out, resuming...\n");

    // if (!stream.httpConnected())
    //     Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

LivingRoom LivingRoom::getLivingRoomObject()
{
    return LivingRoomObject;
}

LivingRoom::LivingRoom()
{
}
LivingRoom::~LivingRoom()
{
}
