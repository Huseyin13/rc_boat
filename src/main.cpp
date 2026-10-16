#include <Arduino.h>
#include "config.h"
#include "lora.h"
#include "imu.h"
#include "gps.h"
#include "can.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Mailbox queues (length 1, each one always holds the latest value)
static QueueHandle_t imuQueue;
static QueueHandle_t gpsQueue;
static QueueHandle_t canQueue;

void imuReaderTask(void *param);
void gpsReaderTask(void *param);
void canReceiverTask(void *param);
void loraTransmitterTask(void *param);

void setup()
{
    Serial.begin(115200);
    delay(200);
    Serial.println("RC-Boat telemetry system starting...");

    // Initialise modules
    imu_init();
    gps_init();
    can_init();
    lora_init();

    // Create queues
    imuQueue = xQueueCreate(1, sizeof(ImuData));
    gpsQueue = xQueueCreate(1, sizeof(GpsData));
    canQueue = xQueueCreate(1, sizeof(CanData));

    // Pin tasks to cores
    // function, name, stack, param, priority, handle, core
    xTaskCreatePinnedToCore(canReceiverTask, "Can", 4096, NULL, 6, NULL, 1);
    xTaskCreatePinnedToCore(imuReaderTask, "Imu", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(loraTransmitterTask, "Lora", 4096, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(gpsReaderTask, "Gps", 4096, NULL, 2, NULL, 0);
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000)); // 1 second
}

void imuReaderTask(void *param)
{
    ImuData imuData;

    for (;;)
    {
        if (imu_read(imuData))
        {
            xQueueOverwrite(imuQueue, &imuData);
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // 20ms
    }
}

void gpsReaderTask(void *param)
{
    GpsData gpsData;

    for (;;)
    {
        if (gps_read(gpsData))
        {
            xQueueOverwrite(gpsQueue, &gpsData);
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms
    }
}

void canReceiverTask(void *param)
{
    CanData canData = {};

    for (;;)
    {
        if (can_read(canData))
        {
            xQueueOverwrite(canQueue, &canData);
        }
    }
}

void loraTransmitterTask(void *param)
{
    ImuData imuData = {};
    GpsData gpsData = {};
    CanData canData = {};

    for (;;)
    {
        // Read the latest values from the queues
        xQueuePeek(imuQueue, &imuData, 0);
        xQueuePeek(gpsQueue, &gpsData, 0);
        xQueuePeek(canQueue, &canData, 0);

        // Send over LoRa
        lora_send(imuData, gpsData, canData);

        vTaskDelay(pdMS_TO_TICKS(200)); // 200ms
    }
}
