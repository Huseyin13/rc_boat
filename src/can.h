#pragma once

#include <Arduino.h> // int32_t, uint8_t, etc.

// Telemetry received from the VESC over CAN.
// (Each CAN packet carries only part of this; see can.cpp for details.)
struct CanData
{
    int32_t erpm;      // motor speed (eRPM)
    float   current;   // motor current (A)
    float   duty;      // throttle / duty cycle (%)
    float   voltage;   // input (battery) voltage (V)
    float   tempFet;   // MOSFET (driver) temperature (°C)
    float   tempMotor; // motor temperature (°C)

    bool    valid;     // whether this reading is valid
};

bool can_init();
bool can_read(CanData &data);
