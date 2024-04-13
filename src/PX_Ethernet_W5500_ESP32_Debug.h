#pragma once

#ifndef PX_ETHERNET_W5500_ESP32_DEBUG_H
#define PX_ETHERNET_W5500_ESP32_DEBUG_H

#include <stdio.h>

#define PX_ETHERNET_SERIAL Serial

// Change _PX_ETHERNET_LOGLEVEL_ to set tracing and logging verbosity
// 0: DISABLED: no logging
// 1: ERROR: errors
// 2: WARN: errors and warnings
// 3: INFO: errors, warnings and informational (default)
// 4: DEBUG: errors, warnings, informational and debug

#ifndef _PX_ETHERNET_LOGLEVEL_
#define _PX_ETHERNET_LOGLEVEL_ 4
#endif

#define PX_ETH_PRINT PX_ETHERNET_SERIAL.print
#define PX_ETH_PRINTLN PX_ETHERNET_SERIAL.println

///////////////////////////////////////

#define PX_ETH_LOGERROR(x)          \
  if (_PX_ETHERNET_LOGLEVEL_ > 0)   \
  {                                 \
    PX_ETH_PRINT("[ERR] PX-ETH: "); \
    PX_ETH_PRINTLN(x);              \
  }

#define PX_ETH_LOGWARN(x)           \
  if (_PX_ETHERNET_LOGLEVEL_ > 1)   \
  {                                 \
    PX_ETH_PRINT("[WRN] PX-ETH: "); \
    PX_ETH_PRINTLN(x);              \
  }

#define PX_ETH_LOGINFO(x)           \
  if (_PX_ETHERNET_LOGLEVEL_ > 2)   \
  {                                 \
    PX_ETH_PRINT("[INF] PX-ETH: "); \
    PX_ETH_PRINTLN(x);              \
  }

#define PX_ETH_LOGDEBUG(x)          \
  if (_PX_ETHERNET_LOGLEVEL_ > 3)   \
  {                                 \
    PX_ETH_PRINT("[LOG] PX-ETH: "); \
    PX_ETH_PRINTLN(x);              \
  }

#endif
