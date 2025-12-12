/**
 * @file hw_config.c
 * @brief Implementación de configuración de hardware (SD + SPI) utilizada por FatFS.
 *
 * Contiene instancias estáticas de spi_t y sd_card_t que describen la
 * configuración física del bus SPI y la tarjeta SD usada por el sistema.
 *
 * Las funciones sd_get_num / sd_get_by_num / spi_get_num / spi_get_by_num
 * son llamadas por la capa FatFS (u otro código) para obtener estas
 * estructuras en tiempo de ejecución.
 *
 * No modifica hardware por sí mismo: solo provee descriptores de configuración.
 *
 * @authors
 *  - Mauricio Reyes Rosero
 *  - Reinaldo Marín Nieto
 *  - Daniel Pérez Gallego
 *  - Jorge Arroyo Niño
 */

#include "hw_config.h"
#include <stdio.h>

/* -------------------------------------------------------------------------
 * Instancia SPI estática (SPI1: GPIO 24-27)
 * ------------------------------------------------------------------------- */
/** @brief Configuración de la instancia SPI usada para la SD. */
static spi_t spi = {
    .hw_inst = spi1,              /**< hardware SPI (ej. spi1) */
    .sck_gpio = SD_CLK_PIN,       /**< GPIO de SCK (CLK) */
    .mosi_gpio = SD_MOSI_PIN,     /**< GPIO de MOSI (TX) */
    .miso_gpio = SD_MISO_PIN,     /**< GPIO de MISO (RX) */
    .baud_rate = SD_SPI_BAUDRATE, /**< velocidad nominal SPI */
    .set_drive_strength = true    /**< ajustar drive strength si aplica */
};

/* -------------------------------------------------------------------------
 * Instancia de tarjeta SD
 * ------------------------------------------------------------------------- */
/** @brief Descriptor de la tarjeta SD conectada al SPI anterior. */
static sd_card_t sd_card = {
    .pcName = "0:",               /**< nombre lógico para FatFS */
    .spi = &spi,                  /**< puntero a la configuración SPI */
    .ss_gpio = SD_CS_PIN,         /**< GPIO del Chip Select */
    .use_card_detect = false,     /**< si hay detección física de tarjeta */
    .card_detect_gpio = -1,       /**< GPIO de detect (si aplica) */
    .card_detected_true = -1      /**< nivel lógico que indica detectada */
};

/* -------------------------------------------------------------------------
 * Funciones requeridas por FatFS / capa de abstracción
 * ------------------------------------------------------------------------- */

/**
 * @brief Retorna el número de tarjetas SD configuradas.
 * @return Cantidad de tarjetas SD disponibles.
 */
size_t sd_get_num() {
    return 1;
}

/**
 * @brief Obtiene la estructura sd_card_t asociada al índice solicitado.
 * @param num Índice de tarjeta (0..sd_get_num()-1).
 * @return Puntero a sd_card_t o NULL si el índice es inválido.
 */
sd_card_t *sd_get_by_num(size_t num) {
    if (num == 0) {
        return &sd_card;
    }
    return NULL;
}

/**
 * @brief Retorna el número de interfaces SPI configuradas.
 * @return Cantidad de instancias SPI disponibles.
 */
size_t spi_get_num() {
    return 1;
}

/**
 * @brief Obtiene la estructura spi_t asociada al índice solicitado.
 * @param num Índice SPI (0..spi_get_num()-1).
 * @return Puntero a spi_t o NULL si el índice es inválido.
 */
spi_t *spi_get_by_num(size_t num) {
    if (num == 0) {
        return &spi;
    }
    return NULL;
}
