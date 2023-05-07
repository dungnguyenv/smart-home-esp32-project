#ifndef COMMONS_H
#define COMMONS_H

#include <Arduino.h>

String concat2String(String s1, String s2)
{
    s1.concat(s2);
    return s1;
}

#endif