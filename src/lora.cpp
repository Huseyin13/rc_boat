#include "lora.h"
#include "config.h" // Pin definitions and other configuration
#include <HardwareSerial.h>

static HardwareSerial LoRaSerial(LORA_UART_NUM); // the LoRa module uses the second UART port

bool lora_init()
{
    // Mode select pins
    pinMode(LORA_M0_PIN, OUTPUT);
    pinMode(LORA_M1_PIN, OUTPUT);

    // Put the LoRa module into normal (transparent) mode (M0=0, M1=0)
    digitalWrite(LORA_M0_PIN, LOW);
    digitalWrite(LORA_M1_PIN, LOW);

    // Start the LoRa serial port
    LoRaSerial.begin(LORA_BAUD, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
    return true;
}

void lora_send(const ImuData &imu_data,
               const GpsData &gps_data,
               const CanData &can_data)
{
    // The ground station (ground_station.py) expects key:value pairs:
    //   E:erpm,C:current,D:duty,S:speed,W:qw,X:qx,Y:qy,Z:qz  (terminated by \n)
    // Order-independent and tolerant to missing/corrupt fields, so it is robust over LoRa.
    String packet = "";

    // --- VESC (dashboard: drive panel) ---
    packet += "E:" + String(can_data.erpm);        // eRPM (int)
    packet += ",C:" + String(can_data.current, 2); // Amps
    packet += ",D:" + String(can_data.duty, 3);    // 0.0 - 1.0 (dashboard converts to %)

    // --- GPS speed (dashboard: main metric) ---
    packet += ",S:" + String(gps_data.speed, 2); // km/h

    // --- IMU quaternion (dashboard: artificial horizon + Euler) ---
    // If no IMU data is available, send the identity quaternion (1,0,0,0):
    // the horizon stays level and the quaternion->Euler conversion does not produce NaN.
    if (imu_data.valid)
    {
        packet += ",W:" + String(imu_data.qw, 4);
        packet += ",X:" + String(imu_data.qx, 4);
        packet += ",Y:" + String(imu_data.qy, 4);
        packet += ",Z:" + String(imu_data.qz, 4);
    }
    else
    {
        packet += ",W:1.0000,X:0.0000,Y:0.0000,Z:0.0000";
    }

    // --- Extra telemetry (dashboard: voltage/power/temperature/position panels) ---
    // The dashboard recognises these; a receiver that does not will silently ignore them.
    packet += ",V:" + String(can_data.voltage, 1);    // battery voltage (V)
    packet += ",TF:" + String(can_data.tempFet, 1);   // FET temperature (°C)
    packet += ",TM:" + String(can_data.tempMotor, 1); // motor temperature (°C)
    packet += ",LA:" + String(gps_data.latitude, 6);  // latitude (6 decimals required)
    packet += ",LO:" + String(gps_data.longitude, 6); // longitude (6 decimals required)
    packet += ",SAT:" + String(gps_data.satellites);  // satellite count

    LoRaSerial.println(packet); // terminate with \n -> the dashboard splits lines this way
}
