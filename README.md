
# PX_Ethernet_W5500_ESP32

### Why do we need this [PX_Ethernet_W5500_ESP32 library](https://github.com/mrgreen007/PX_Ethernet_W5500_ESP32)

This library is inspired from Arduino's Ethernet library. In ESP32 we can not directly use that lib with existing WiFi stack. By using this library we can run both WiFi & Ethernet functionality at the same time under same subnet.

---

#### Currently supported Boards

1. **(ESP32 + W5500 Ethernet) boards**

---
---


## Prerequisites

 1. [`Arduino IDE 1.8.19+` for Arduino](https://github.com/arduino/Arduino). [![GitHub release](https://img.shields.io/github/release/arduino/Arduino.svg)](https://github.com/arduino/Arduino/releases/latest)
 2. [`ESP32 Core 2.0.6+`](https://github.com/espressif/arduino-esp32) for ESP32-based boards. ESP32 Latest Core [![Latest release](https://img.shields.io/github/release/espressif/arduino-esp32.svg)](https://github.com/espressif/arduino-esp32/releases/latest/)

---

## Installation

### Manual Install

1. Navigate to [PX_Ethernet_W5500_ESP32](https://github.com/mrgreen007/PX_Ethernet_W5500_ESP32) page.
2. Download the latest release `PX_Ethernet_W5500_ESP32-main.zip`.
3. Extract the zip file to `PX_Ethernet_W5500_ESP32-main` directory 
4. Copy the whole `PX_Ethernet_W5500_ESP32-main` folder to Arduino libraries' directory such as `~/Arduino/libraries/`.

---
---

## Usages

```cpp
// To get MAC address used by WiFi itself 
byte  mac[6];
WiFi.macAddress(mac);

// To be called before ETH.begin()
ESP32_W5500_onEvent();

// To begin the Ethernet. By default it get config from DHCP
ETH.begin(MISO_GPIO, MOSI_GPIO, SCK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac);

// For static IP configuration (Optional)
ETH.config(static_ip, gateway, subnet, dns)

```

---
---

### How to connect W5500 to ESP32

You can change the `INT` pin to another one. Default is `GPIO4`

```cpp
// Must connect INT to GPIOxx or not working
#define INT_GPIO            4
```

---

#### W5500

<p align="center">
    <img src="https://robu.in/wp-content/uploads/2019/01/SPI-to-Ethernet-Hardware-TCPIP-W5500-Ethernet-Network-Module-8.jpg">
</p>
 
--- 
 

|W5500|<--->|ESP32|
|:-:|:-:|:-:|
|MOSI|<--->|GPIO23|
|MISO|<--->|GPIO19|
|SCK|<--->|GPIO18|
|SS|<--->|GPIO5|
|INT|<--->|GPIO4|
|RST|<--->|RST|
|GND|<--->|GND|
|3.3V|<--->|3.3V|


---
---

### Debug

Debug is enabled by default on Serial. Debug Level from 0 to 4. To disable, change the `_PX_ETHERNET_LOGLEVEL_` to 0

```cpp
// Use this to disable all output debug msgs
// Debug Level from 0 to 4
#define _PX_ETHERNET_LOGLEVEL_       0
```

---

### Troubleshooting

If you get compilation errors, more often than not, you may need to install a newer version of Arduino IDE, the Arduino `ESP32` core or depending libraries.

Sometimes, the library will only work if you update the `ESP32` core to the latest version because I'm always using the latest cores /libraries.

---

### Issues ###

Submit issues to: [PX_Ethernet_W5500_ESP32 issues](https://github.com/mrgreen007/PX_Ethernet_W5500_ESP32/issues)

---

### Contributing

If you want to contribute to this project:
- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library

---