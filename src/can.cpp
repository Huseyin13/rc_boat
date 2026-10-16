#include "can.h"
#include "config.h"      // Pin definitions and other configuration
#include "driver/twai.h" // ESP32 native CAN (TWAI) driver - no extra library needed

bool can_init()
{
    // General driver config (listen-only: we never transmit on the VESC bus)
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN_TX_PIN, (gpio_num_t)CAN_RX_PIN, TWAI_MODE_LISTEN_ONLY);
    // Bit timing
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    // Acceptance filter (accept all messages)
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Install the driver
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK)
    {
        Serial.println("CAN driver install failed: twai_driver_install() returned an error!");
        return false;
    }

    if (twai_start() != ESP_OK)
    {
        Serial.println("CAN driver start failed: twai_start() returned an error!");
        return false;
    }
    return true;
}

bool can_read(CanData &data)
{
    twai_message_t message;

    // Try to receive a message with a short timeout
    if (twai_receive(&message, pdMS_TO_TICKS(10)) != ESP_OK)
    {
        Serial.println("No CAN message received: twai_receive() returned an error!");
        return false;
    }

    // VESC status packets always use extended IDs, so check for that
    if (!message.extd)
    {
        Serial.println("CAN message is not an extended ID, skipped!");
        return false;
    }

    // Upper byte of the ID is the packet type, lower byte is the controller ID (we have a single VESC)
    uint8_t packet_type = (message.identifier >> 8) & 0xFF;

    switch (packet_type)
    {
    case 9:
    {
        // Status_1 -> eRPM in data[0..3]
        int32_t erpm = (int32_t)((message.data[0] << 24) | (message.data[1] << 16) | (message.data[2] << 8) | message.data[3]);
        data.erpm = erpm;
        int16_t curr = (int16_t)((message.data[4] << 8) | message.data[5]);
        data.current = curr / 10.0f; // A
        int16_t d = (int16_t)((message.data[6] << 8) | message.data[7]);
        data.duty = d / 1000.0f; // %
        break;
    }
    case 16:
    {
        // Status_4 -> temperatures in data[0..3]
        int16_t tempF = (int16_t)((message.data[0] << 8) | message.data[1]);
        data.tempFet = tempF / 10.0f;
        int16_t tempMotor = (int16_t)((message.data[2] << 8) | message.data[3]);
        data.tempMotor = tempMotor / 10.0f; // °C
        break;
    }
    case 27: // Status_5 -> input voltage in data[4..5]
    {
        int16_t v = (int16_t)((message.data[4] << 8) | message.data[5]);
        data.voltage = v / 10.0f; // V
        break;
    }
    }
    data.valid = true; // This reading is valid
    return true;
}
