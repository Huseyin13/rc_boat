#pragma once
#include <Arduino.h> // uint8_t

struct GpsData
{
    double latitude;    // 8 bytes
    double longitude;   // 8 bytes
    float speed;        // 4 bytes
    uint8_t satellites; // 1 byte (satellite count 0..255, never negative)
    bool valid;         // 1 byte
};

bool gps_init();
bool gps_read(GpsData &data);
