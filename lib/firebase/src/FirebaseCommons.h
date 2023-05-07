#ifndef FirebaseCommons_H
#define FirebaseCommons_H

#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include "ConnectionConfig.h"

void firebaseConfig(FirebaseConfig &config, FirebaseAuth &auth)
{
    config.api_key = API_KEY;

    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    config.database_url = DATABASE_URL;

    config.token_status_callback = tokenStatusCallback;
}



#endif