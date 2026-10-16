#pragma once

// ====== I2C — BNO085 IMU ======
#define BNO085_SDA_PIN      21
#define BNO085_SCL_PIN      22
#define BNO085_INT_PIN      23
#define BNO085_RST_PIN      19
#define BNO085_I2C_ADDR     0x4A   // ADR low; 0x4B if high

// ====== UART1 — NEO GPS ======
#define GPS_RX_PIN          27     // ESP32 RX  <- GPS TX
#define GPS_TX_PIN          14     // ESP32 TX  -> GPS RX
#define GPS_BAUD            9600
#define GPS_UART_NUM        1

// ====== UART2 — EBYTE E220 LoRa ======
#define LORA_RX_PIN         16     // ESP32 RX2 <- LoRa TX
#define LORA_TX_PIN         17     // ESP32 TX2 -> LoRa RX
#define LORA_M0_PIN         32
#define LORA_M1_PIN         33
#define LORA_AUX_PIN        25
#define LORA_BAUD           9600
#define LORA_UART_NUM       2

// ====== TWAI / CAN — SN65HVD230 (VESC) ======
#define CAN_TX_PIN          5      // -> CTX
#define CAN_RX_PIN          4      // -> CRX
#define CAN_BITRATE_KBPS    500    // VESC default
