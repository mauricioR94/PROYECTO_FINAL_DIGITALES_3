#include "hw_config.h"
#include <stdio.h>

// Configuración del periférico SPI para la SD Card
static spi_t spi = {
    .hw_inst = spi0,           // Usar SPI0
    .sck_gpio = 2,             // GPIO 2 = CLK
    .mosi_gpio = 3,            // GPIO 3 = MOSI
    .miso_gpio = 4,            // GPIO 4 = MISO
    .baud_rate = 12500 * 1000, // 12.5 MHz
    .set_drive_strength = false
};

// Configuración de la tarjeta SD
static sd_card_t sd_card = {
    .pcName = "0:",            // Nombre del volumen
    .spi = &spi,               // Apuntador al SPI configurado
    .ss_gpio = 5,              // GPIO 5 = CS (Chip Select)
    .use_card_detect = false,  // No usamos detección de tarjeta
    .card_detect_gpio = -1,
    .card_detected_true = -1
};

// ========================================
// Funciones requeridas por FatFS (SD Card)
// ========================================
size_t sd_get_num() {
    return 1;  // Solo tenemos una tarjeta SD
}

sd_card_t *sd_get_by_num(size_t num) {
    if (num == 0) {
        return &sd_card;
    }
    return NULL;
}

// ========================================
// Funciones requeridas por el driver SPI
// ========================================
size_t spi_get_num() {
    return 1;  // Solo tenemos un bus SPI configurado
}

spi_t *spi_get_by_num(size_t num) {
    if (num == 0) {
        return &spi;
    }
    return NULL;
}

// ========================================
// Funciones auxiliares para la aplicación
// ========================================
bool sd_init() {
    printf("Inicializando tarjeta SD...\n");
    
    sd_card_t *pSD = sd_get_by_num(0);
    if (!pSD) {
        printf("Error: No se encontró configuración de SD\n");
        return false;
    }
    
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (fr != FR_OK) {
        printf("Error al montar SD (código: %d)\n", fr);
        return false;
    }
    
    printf("SD montada correctamente en '%s'\n", pSD->pcName);
    return true;
}

void sd_deinit() {
    sd_card_t *pSD = sd_get_by_num(0);
    if (pSD) {
        f_unmount(pSD->pcName);
        printf("SD desmontada\n");
    }
}