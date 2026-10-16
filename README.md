# RC Boat Telemetry (ESP32)

Onboard telemetry system for a remote-controlled catamaran, built as the small-scale
prototype stage of the **Turquoise Racing Team (RWTH Aachen)** on the way to the
**Monaco Energy Boat Challenge**. The goal is to validate the sensing, motor-data and
radio-link concepts at small scale before moving to the full-size energy boat.

The ESP32 does not take part in driving the boat. It only collects data from the IMU, the
GPS and the motor controller (VESC), and streams it to a ground station over LoRa.

## Hardware

| Component | Role | Interface |
|---|---|---|
| ESP32-WROOM-32 DevKit V1 | Central controller, telemetry collection | - |
| Bosch BNO085 IMU | Orientation (quaternion / Euler) with on-chip sensor fusion | I2C |
| u-blox NEO series GPS | Speed and position | UART1 |
| Flipsky VESC 75100 | Motor controller; source of eRPM, current, duty, voltage, temperatures | CAN |
| TI SN65HVD230 | 3.3 V CAN transceiver | TWAI (ESP32 native CAN) |
| EBYTE E220 LoRa | Long-range telemetry link to shore | UART2 |
| DUMBORC P6F receiver | Steering (CH1, servo) and throttle (CH2, VESC) | PWM, independent of the ESP32 |

Drive and steering stay on the RC receiver, so a firmware fault on the ESP32 can never
affect control of the boat.

### Pin map

All pins and constants live in [`include/config.h`](include/config.h).

| Signal | GPIO |
|---|---|
| BNO085 SDA / SCL / INT / RST | 21 / 22 / 23 / 19 |
| GPS RX / TX | 27 / 14 |
| LoRa RX / TX / M0 / M1 / AUX | 16 / 17 / 32 / 33 / 25 |
| CAN TX / RX | 5 / 4 |

## Software architecture

The firmware is written with the Arduino framework on top of FreeRTOS. Each peripheral has
its own driver module (`imu`, `gps`, `can`, `lora`) and its own task. Tasks exchange data
through length-1 "mailbox" queues (`xQueueOverwrite` / `xQueuePeek`), so the consumer always
sees the most recent value and a slow producer never blocks anyone.

| Task | Core | Priority | Period | Job |
|---|---|---|---|---|
| `canReceiverTask` | 1 | 6 | event-driven | Listen to VESC status frames |
| `imuReaderTask` | 1 | 5 | 20 ms | Read BNO085 rotation vector |
| `loraTransmitterTask` | 0 | 3 | 200 ms | Assemble and send the telemetry packet |
| `gpsReaderTask` | 0 | 2 | 100 ms | Parse NMEA sentences |

Time-critical sensor reading runs on core 1; latency-tolerant parsing and radio work run
on core 0.

### VESC over CAN

The TWAI controller runs in **listen-only** mode at 500 kbit/s, so the ESP32 never puts a
frame on the motor-controller bus. VESC status frames use 29-bit extended IDs; the upper
byte is the packet type and the lower byte the controller ID:

| Packet | ID | Decoded fields |
|---|---|---|
| `STATUS_1` | 9 | eRPM, motor current, duty cycle |
| `STATUS_4` | 16 | FET temperature, motor temperature |
| `STATUS_5` | 27 | input voltage |

Frame layouts follow the VESC firmware sources (`vedderb/bldc`).

### Telemetry packet

Each LoRa packet is a single line of comma-separated `key:value` pairs terminated by `\n`:

```
E:12000,C:15.20,D:0.450,S:18.30,W:0.9990,X:0.0100,Y:0.0200,Z:0.0300,V:22.4,TF:41.0,TM:38.5,LA:50.776350,LO:6.083870,SAT:9
```

| Key | Meaning | Unit |
|---|---|---|
| `E` | motor speed | eRPM |
| `C` | motor current | A |
| `D` | duty cycle | 0.0-1.0 |
| `S` | GPS speed | km/h |
| `W` `X` `Y` `Z` | orientation quaternion | - |
| `V` | battery voltage | V |
| `TF` / `TM` | FET / motor temperature | °C |
| `LA` / `LO` | latitude / longitude | degrees |
| `SAT` | satellites in use | - |

The format is order-independent and the receiver ignores unknown or corrupt fields, which
keeps it robust over a lossy radio link and lets new keys be added without breaking older
ground-station software. If no IMU data is available the identity quaternion is sent, so
the attitude display stays level instead of producing NaN.

## Project structure

```
include/config.h   pins and constants (single source of truth)
src/main.cpp       FreeRTOS setup, queues and tasks
src/imu.*          BNO085 driver wrapper
src/gps.*          GPS reader (TinyGPSPlus)
src/can.*          VESC CAN listener (ESP-IDF TWAI driver)
src/lora.*         E220 LoRa transmitter and packet encoding
```

## Build

Requires [PlatformIO](https://platformio.org/).

```
pio run                 # build
pio run -t upload       # flash
pio device monitor      # serial monitor (115200 baud)
```

Libraries (installed automatically from `platformio.ini`): SparkFun BNO080, TinyGPSPlus,
EByte LoRa E220.

## Status

All four driver modules and the FreeRTOS integration are implemented and build. The
modules are being tested on the bench one by one; the complete system has not yet been
tested on the water.
