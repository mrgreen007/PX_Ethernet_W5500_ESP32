#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "driver/gpio.h"
#include "esp_eth_w5500.h"
#include "driver/spi_master.h"

#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "w5500.spi";

////////////////////////////////////////

esp_eth_mac_t* w5500_new_mac( spi_device_handle_t *spi_handle, int INT_GPIO )
{
  eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG( *spi_handle );
  w5500_config.int_gpio_num = INT_GPIO;

  eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();

  //eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
  //phy_config.reset_gpio_num = -1;

  mac_config.smi_mdc_gpio_num  = -1; // w5500 doesn't have SMI interface
  mac_config.smi_mdio_gpio_num = -1;
  mac_config.rx_task_prio      = 1;

  return esp_eth_mac_new_w5500( &w5500_config, &mac_config );
}

////////////////////////////////////////

esp_eth_mac_t* w5500_begin(int MISO_GPIO, int MOSI_GPIO, int SCLK_GPIO, int CS_GPIO, int INT_GPIO, int SPICLOCK_MHZ,
                           int SPIHOST)
{
  if (ESP_OK != gpio_install_isr_service(0))
  {
    ESP_LOGE(TAG, "%s(%d): Error gpio_install_isr_service", __FUNCTION__, __LINE__);

    return NULL;
  }

  /* w5500 ethernet driver is based on spi driver */
  spi_bus_config_t buscfg =
  {
    .miso_io_num   = MISO_GPIO,
    .mosi_io_num   = MOSI_GPIO,
    .sclk_io_num   = SCLK_GPIO,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
  };

  if ( ESP_OK != spi_bus_initialize( SPIHOST, &buscfg, 1 ))
  {
    ESP_LOGE(TAG, "%s(%d): Error spi_bus_initialize", __FUNCTION__, __LINE__);

    return NULL;
  }

  spi_device_interface_config_t devcfg =
  {
    .command_bits = 16,
    .address_bits = 8,
    .mode = 0,
    .clock_speed_hz = SPICLOCK_MHZ * 1000 * 1000,
    .spics_io_num = CS_GPIO,
    .queue_size = 20,
    .cs_ena_posttrans = w5500_cal_spi_cs_hold_time(SPICLOCK_MHZ),
  };

  spi_device_handle_t spi_handle = NULL;

  if (ESP_OK != spi_bus_add_device( SPIHOST, &devcfg, &spi_handle ))
  {
    ESP_LOGE(TAG, "%s(%d): Error spi_bus_add_device", __FUNCTION__, __LINE__);

    return NULL;
  }

  return w5500_new_mac( &spi_handle, INT_GPIO );
}

////////////////////////////////////////

