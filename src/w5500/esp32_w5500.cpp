#include "PX_Ethernet_W5500_ESP32_Debug.h"
#include "esp32_w5500.h"

extern "C"
{
  esp_eth_mac_t *w5500_begin(int MISO, int MOSI, int SCLK, int CS, int INT, int SPICLOCK_MHZ,
                             int SPIHOST);
#include "esp_eth/esp_eth_w5500.h"
}

#include "esp_event.h"
#include "esp_eth_phy.h"
#include "esp_eth_mac.h"
#include "esp_eth_com.h"

#if CONFIG_IDF_TARGET_ESP32
#include "soc/emac_ext_struct.h"
#include "soc/rtc.h"
#endif

#include "lwip/err.h"
#include "lwip/dns.h"

extern void tcpipInit();

////////////////////////////////////////

ESP32_W5500::ESP32_W5500()
    : initialized(false), staticIP(false), eth_handle(NULL), started(false), eth_link(ETH_LINK_DOWN)
{
}

////////////////////////////////////////

ESP32_W5500::~ESP32_W5500()
{
}

////////////////////////////////////////

bool ESP32_W5500::begin(int MISO, int MOSI, int SCLK, int CS, int INT, int SPICLOCK_MHZ, int SPIHOST,
                        uint8_t *W5500_Mac)
{
  tcpipInit();

  esp_base_mac_addr_set(W5500_Mac);

  tcpip_adapter_set_default_eth_handlers();

  esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
  esp_netif_t *eth_netif = esp_netif_new(&cfg);

  esp_eth_mac_t *eth_mac = w5500_begin(MISO, MOSI, SCLK, CS, INT, SPICLOCK_MHZ, SPIHOST);

  if (eth_mac == NULL)
  {
    PX_ETH_LOGERROR(F("esp_eth_mac_new_esp32 failed"));

    return false;
  }

  eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
  phy_config.autonego_timeout_ms = 0; // W5500 doesn't support auto-negotiation
  phy_config.reset_gpio_num = -1;     // W5500 doesn't have a pin to reset internal PHY
  esp_eth_phy_t *eth_phy = esp_eth_phy_new_w5500(&phy_config);

  if (eth_phy == NULL)
  {
    PX_ETH_LOGERROR(F("esp_eth_phy_new failed"));

    return false;
  }

  eth_handle = NULL;
  esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(eth_mac, eth_phy);

  if (esp_eth_driver_install(&eth_config, &eth_handle) != ESP_OK || eth_handle == NULL)
  {
    PX_ETH_LOGDEBUG(F("esp_eth_driver_install failed"));

    return false;
  }

  eth_mac->set_addr(eth_mac, W5500_Mac);

#if 1

  if ((SPICLOCK_MHZ < 14) || (SPICLOCK_MHZ > 25))
  {
    PX_ETH_LOGERROR(F("SPI Clock must be >= 8 and <= 25 MHz for W5500"));
    ESP_ERROR_CHECK(ESP_FAIL);
  }

#endif

  /* attach Ethernet driver to TCP/IP stack */
  if (esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)) != ESP_OK)
  {
    PX_ETH_LOGERROR(F("esp_netif_attach failed"));

    return false;
  }

  if (esp_eth_start(eth_handle) != ESP_OK)
  {
    PX_ETH_LOGDEBUG(F("esp_eth_start failed"));

    return false;
  }

  // holds a few microseconds to let DHCP start and enter into a good state
  // FIX ME -- addresses issue https://github.com/espressif/arduino-esp32/issues/5733
  delay(50);

  return true;
}

////////////////////////////////////////

bool ESP32_W5500::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2)
{
  esp_err_t err = ESP_OK;
  tcpip_adapter_ip_info_t info;

  if (static_cast<uint32_t>(local_ip) != 0)
  {
    info.ip.addr = static_cast<uint32_t>(local_ip);
    info.gw.addr = static_cast<uint32_t>(gateway);
    info.netmask.addr = static_cast<uint32_t>(subnet);
  }
  else
  {
    info.ip.addr = 0;
    info.gw.addr = 0;
    info.netmask.addr = 0;
  }

  err = tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_ETH);

  if (err != ESP_OK && err != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPPED)
  {
    PX_ETH_LOGERROR(F("DHCP could not be stopped! Error ="));
    PX_ETH_LOGERROR(err);
    return false;
  }

  err = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_ETH, &info);

  if (err != ERR_OK)
  {
    PX_ETH_LOGERROR(F("STA IP could not be configured! Error = "));
    PX_ETH_LOGERROR(err);
    return false;
  }

  if (info.ip.addr)
  {
    staticIP = true;
  }
  else
  {
    err = tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_ETH);

    if (err != ESP_OK && err != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STARTED)
    {
      PX_ETH_LOGWARN(F("DHCP could not be started! Error ="));
      PX_ETH_LOGERROR(err);
      return false;
    }

    staticIP = false;
  }

  ip_addr_t d;
  d.type = IPADDR_TYPE_V4;

  if (static_cast<uint32_t>(dns1) != 0)
  {
    // Set DNS1-Server
    d.u_addr.ip4.addr = static_cast<uint32_t>(dns1);
    dns_setserver(0, &d);
  }

  if (static_cast<uint32_t>(dns2) != 0)
  {
    // Set DNS2-Server
    d.u_addr.ip4.addr = static_cast<uint32_t>(dns2);
    dns_setserver(1, &d);
  }

  return true;
}

////////////////////////////////////////

IPAddress ESP32_W5500::localIP()
{
  tcpip_adapter_ip_info_t ip;

  if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
  {
    PX_ETH_LOGDEBUG(F("localIP NULL"));

    return IPAddress();
  }

  // PX_ETH_LOGDEBUG(IPAddress(ip.ip.addr));

  return IPAddress(ip.ip.addr);
}

////////////////////////////////////////

IPAddress ESP32_W5500::subnetMask()
{
  tcpip_adapter_ip_info_t ip;

  if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
  {
    return IPAddress();
  }

  return IPAddress(ip.netmask.addr);
}

////////////////////////////////////////

IPAddress ESP32_W5500::gatewayIP()
{
  tcpip_adapter_ip_info_t ip;

  if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
  {
    return IPAddress();
  }

  return IPAddress(ip.gw.addr);
}

////////////////////////////////////////

IPAddress ESP32_W5500::dnsIP(uint8_t dns_no)
{
  const ip_addr_t *dns_ip = dns_getserver(dns_no);

  return IPAddress(dns_ip->u_addr.ip4.addr);
}

////////////////////////////////////////

IPAddress ESP32_W5500::broadcastIP()
{
  tcpip_adapter_ip_info_t ip;

  if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
  {
    return IPAddress();
  }

  return WiFiGenericClass::calculateBroadcast(IPAddress(ip.gw.addr), IPAddress(ip.netmask.addr));
}

////////////////////////////////////////

IPAddress ESP32_W5500::networkID()
{
  tcpip_adapter_ip_info_t ip;

  if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
  {
    return IPAddress();
  }

  return WiFiGenericClass::calculateNetworkID(IPAddress(ip.gw.addr), IPAddress(ip.netmask.addr));
}

////////////////////////////////////////

uint8_t ESP32_W5500::subnetCIDR()
{
  tcpip_adapter_ip_info_t ip;

  if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
  {
    return (uint8_t)0;
  }

  return WiFiGenericClass::calculateSubnetCIDR(IPAddress(ip.netmask.addr));
}

////////////////////////////////////////

const char *ESP32_W5500::getHostname()
{
  const char *hostname;

  if (tcpip_adapter_get_hostname(TCPIP_ADAPTER_IF_ETH, &hostname))
  {
    return NULL;
  }

  return hostname;
}

////////////////////////////////////////

bool ESP32_W5500::setHostname(const char *hostname)
{
  return tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_ETH, hostname) == 0;
}

////////////////////////////////////////

bool ESP32_W5500::fullDuplex()
{
#ifdef ESP_IDF_VERSION_MAJOR
  return true; // todo: do not see an API for this
#else
  return eth_config.phy_get_duplex_mode();
#endif
}

////////////////////////////////////////

bool ESP32_W5500::linkUp()
{
#ifdef ESP_IDF_VERSION_MAJOR
  return eth_link == ETH_LINK_UP;
#else
  return eth_config.phy_check_link();
#endif
}

////////////////////////////////////////

uint8_t ESP32_W5500::linkSpeed()
{
#ifdef ESP_IDF_VERSION_MAJOR
  eth_speed_t link_speed;
  esp_eth_ioctl(eth_handle, ETH_CMD_G_SPEED, &link_speed);
  return (link_speed == ETH_SPEED_10M) ? 10 : 100;
#else
  return eth_config.phy_get_speed_mode() ? 100 : 10;
#endif
}

////////////////////////////////////////

bool ESP32_W5500::enableIpV6()
{
  return tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_ETH) == 0;
}

////////////////////////////////////////

IPv6Address ESP32_W5500::localIPv6()
{
  static ip6_addr_t addr;

  if (tcpip_adapter_get_ip6_linklocal(TCPIP_ADAPTER_IF_ETH, &addr))
  {
    return IPv6Address();
  }

  return IPv6Address(addr.addr);
}

////////////////////////////////////////

uint8_t *ESP32_W5500::macAddress(uint8_t *mac)
{
  if (!mac)
  {
    return NULL;
  }

#ifdef ESP_IDF_VERSION_MAJOR
  esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac);
#else
  esp_eth_get_mac(mac);
#endif

  return mac;
}

////////////////////////////////////////

String ESP32_W5500::macAddress()
{
  uint8_t mac[6] = {0, 0, 0, 0, 0, 0};
  char macStr[18] = {0};
  macAddress(mac);
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  return String(macStr);
}

////////////////////////////////////////

ESP32_W5500 ETH;
