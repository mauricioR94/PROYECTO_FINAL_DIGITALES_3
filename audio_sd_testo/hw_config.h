#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#include "ff.h"
#include "sd_card.h"
#include <stdbool.h>

// ========================================
// PINES DEL MÓDULO SD (NUEVAS CONEXIONES)
// ========================================
#define SD_MISO_PIN  28  // GPIO 28
#define SD_CS_PIN    22  // GPIO 22 (Chip Select)
#define SD_CLK_PIN   26  // GPIO 26 (SPI1 SCK)
#define SD_MOSI_PIN  27  // GPIO 27 (SPI1 TX)

// ========================================
// PINES DEL DAC I2S (UDA1334A)
// ========================================
#define I2S_BCLK_PIN  10  // Bit Clock
#define I2S_LRCK_PIN  11  // Left/Right Clock (Word Select)
#define I2S_DIN_PIN   12  // Data Input

// ========================================
// CONFIGURACIÓN SPI PARA SD
// ========================================
#define SD_SPI_BAUDRATE  (25 * 1000 * 1000)  // 25 MHz

// Funciones requeridas por FatFS
size_t sd_get_num();
sd_card_t *sd_get_by_num(size_t num);
size_t spi_get_num();
spi_t *spi_get_by_num(size_t num);

#endif // HW_CONFIG_H