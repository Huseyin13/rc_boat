#include "gps.h"
#include "config.h"
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

static HardwareSerial gpsSerial(GPS_UART_NUM); // UART number from config.h
static TinyGPSPlus gps;

bool gps_init()
{
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    return true;
}

bool gps_read(GpsData &data)
{
    // 1) Always feed every incoming byte to the parser (that is the only way a fix is formed)
    while (gpsSerial.available() > 0)
    {
        gps.encode(gpsSerial.read());
    }

    // 2) The decision depends on VALIDITY, not on whether bytes arrived:
    //    if there is no valid, freshly updated location, report "no data".
    if (!gps.location.isValid() || !gps.location.isUpdated())
    {
        return false;
    }

    // 3) Data is good -> fill the struct
    data.latitude = gps.location.lat();
    data.longitude = gps.location.lng();
    data.speed = gps.speed.kmph();
    data.satellites = gps.satellites.value();
    data.valid = true;

    return true;
}
