#include "hw_config.h"
#include <stdio.h>

// ========================================
// CONFIGURACIÓN DEL PERIFÉRICO SPI1
// ========================================
static spi_t spi = {
    .hw_inst = spi1,              // Usar SPI1 (pines GPIO 24-27)
    .sck_gpio = SD_CLK_PIN,       // GPIO 26 = CLK
    .mosi_gpio = SD_MOSI_PIN,     // GPIO 27 = MOSI
    .miso_gpio = SD_MISO_PIN,     // GPIO 28 = MISO
    .baud_rate = SD_SPI_BAUDRATE, // 25 MHz
    .set_drive_strength = true
};

// ========================================
// CONFIGURACIÓN DE LA TARJETA SD
// ========================================
static sd_card_t sd_card = {
    .pcName = "0:",
    .spi = &spi,
    .ss_gpio = SD_CS_PIN,         // GPIO 22 = CS
    .use_card_detect = false,
    .card_detect_gpio = -1,
    .card_detected_true = -1
};

// ========================================
// FUNCIONES REQUERIDAS POR FATFS
// ========================================
size_t sd_get_num() {
    return 1;
}

sd_card_t *sd_get_by_num(size_t num) {
    if (num == 0) {
        return &sd_card;
    }
    return NULL;
}

size_t spi_get_num() {
    return 1;
}

spi_t *spi_get_by_num(size_t num) {
    if (num == 0) {
        return &spi;
    }
    return NULL;
}