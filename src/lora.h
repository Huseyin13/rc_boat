#pragma once

#include "gps.h"
#include "imu.h"
#include "can.h"

bool lora_init();
void lora_send(
    const ImuData &imu_data,
    const GpsData &gps_data,
    const CanData &can_data);