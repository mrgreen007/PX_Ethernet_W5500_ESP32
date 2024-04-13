#ifndef _PX_ETHERNET_W5500_ESP32_H
#define _PX_ETHERNET_W5500_ESP32_H

#if ((defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 2)) && (ARDUINO_ESP32_GIT_VER != 0x46d5afb1))
#define USING_CORE_ESP32_CORE_V200_PLUS true

#define PX_ETHERNET_W5500_ESP32_VERSION "v1.0.0 for core v2.0.0+"
#else

#define PX_ETHERNET_W5500_ESP32_VERSION "v1.0.0 for core v1.0.6-"
#endif

/////////////////////////////////////////////////

#include "Arduino.h"

#include <functional>
#include "FS.h"

//////////////////////////////////////////////////////////////
// ESP32_W5500 related code

#include "PX_Ethernet_W5500_ESP32_Debug.h"

#include <WiFi.h>

#include "w5500/esp32_w5500.h"

#include <hal/spi_types.h>

#if !defined(ETH_SPI_HOST)
#define ETH_SPI_HOST SPI3_HOST
#endif

#if !defined(SPI_CLOCK_MHZ)
// Using 25MHz for W5500, 14MHz for W5100
#define SPI_CLOCK_MHZ 25
#endif

#if !defined(INT_GPIO)
#define INT_GPIO 4
#endif

#if !defined(MISO_GPIO)
#define MISO_GPIO 19
#endif

#if !defined(MOSI_GPIO)
#define MOSI_GPIO 23
#endif

#if !defined(SCK_GPIO)
#define SCK_GPIO 18
#endif

#if !defined(CS_GPIO)
#define CS_GPIO 5
#endif

#ifndef SHIELD_TYPE
#define SHIELD_TYPE "ESP32_W5500"
#endif

extern bool ESP32_W5500_eth_connected;

extern void ESP32_W5500_onEvent();

extern void ESP32_W5500_waitForConnect();

extern bool ESP32_W5500_isConnected();

extern void ESP32_W5500_event(WiFiEvent_t event);

#endif
