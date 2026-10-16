#pragma once

#include <Arduino.h>

struct ImuData
{
    float qw;
    float qx;
    float qy;
    float qz;

    float yaw;
    float pitch;
    float roll;

    uint8_t accuracy;

    bool valid;
};

bool imu_init();
bool imu_read(ImuData &data);