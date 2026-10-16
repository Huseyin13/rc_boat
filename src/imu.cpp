#include "imu.h"
#include "config.h"
#include <Wire.h>                            // I2C library
#include "SparkFun_BNO080_Arduino_Library.h" // BNO080 library

static BNO080 imu;

static void imu_hardware_reset()
{
    pinMode(BNO085_RST_PIN, OUTPUT);
    digitalWrite(BNO085_RST_PIN, LOW); // reset the chip
    delay(10);
    digitalWrite(BNO085_RST_PIN, HIGH); // release, let it wake up
    delay(50);                          // give it time to boot
}

bool imu_init()
{
    imu_hardware_reset();

    Wire.begin(BNO085_SDA_PIN, BNO085_SCL_PIN); // start I2C on our own pins
    Wire.setClock(400000);                      // 400 kHz

    // Return false if the sensor does not respond at that address.
    if (imu.begin(BNO085_I2C_ADDR, Wire) == false)
    {
        return false;
    }

    imu.enableRotationVector(20); // request a quaternion report every 20 ms
    return true;
}

bool imu_read(ImuData &data)
{
    if (imu.dataAvailable() == false)
    {
        return false; // no new data
    }

    // Quaternion (Real = w, I/J/K = x/y/z)
    data.qw = imu.getQuatReal();
    data.qx = imu.getQuatI();
    data.qy = imu.getQuatJ();
    data.qz = imu.getQuatK();

    data.accuracy = imu.getQuatAccuracy(); // 0..3

    // Euler: the library returns radians -> convert to degrees
    data.roll = imu.getRoll() * 180.0f / PI;
    data.pitch = imu.getPitch() * 180.0f / PI;
    data.yaw = imu.getYaw() * 180.0f / PI;

    data.valid = true;
    return true;
}
