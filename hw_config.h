/**
 * @file hw_config.h
 * @brief Configuración de pines y prototipos de funciones para la integración SD/SPI.
 *
 * Define pines usados por el módulo SD y pines I2S del proyecto, además de
 * las funciones de acceso requeridas por FatFS para enumerar SPI y tarjetas SD.
 *
 * @authors
 *  - Mauricio Reyes Rosero
 *  - Reinaldo Marín Nieto
 *  - Daniel Pérez Gallego
 *  - Jorge Arroyo Niño
 */

#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#include "ff.h"
#include "sd_card.h"
#include <stdbool.h>

/** @brief PIN: MISO del módulo SD (GPIO 28). */
#define SD_MISO_PIN  28

/** @brief PIN: Chip Select del módulo SD (GPIO 22). */
#define SD_CS_PIN    22

/** @brief PIN: SCK del SPI usado por SD (GPIO 26). */
#define SD_CLK_PIN   26

/** @brief PIN: MOSI del SPI usado por SD (GPIO 27). */
#define SD_MOSI_PIN  27

/** @brief PIN: Bit Clock del interface I2S. */
#define I2S_BCLK_PIN  10

/** @brief PIN: LR Clock (Word Select) del interface I2S. */
#define I2S_LRCK_PIN  11

/** @brief PIN: Data In del interface I2S (conectado al DAC). */
#define I2S_DIN_PIN   12

/** @brief Baudrate SPI usado para la tarjeta SD (25 MHz). */
#define SD_SPI_BAUDRATE  (25 * 1000 * 1000)

/**
 * @brief Devuelve la cantidad de tarjetas SD configuradas en el sistema.
 * @return Número de tarjetas SD disponibles (>=1).
 */
size_t sd_get_num();

/**
 * @brief Devuelve la configuración de la tarjeta SD por índice.
 * @param num Índice de tarjeta (0..sd_get_num()-1).
 * @return Puntero a sd_card_t si existe, NULL si índice inválido.
 */
sd_card_t *sd_get_by_num(size_t num);

/**
 * @brief Devuelve la cantidad de buses SPI configurados.
 * @return Número de instancias SPI disponibles.
 */
size_t spi_get_num();

/**
 * @brief Devuelve la configuración SPI por índice.
 * @param num Índice de SPI (0..spi_get_num()-1).
 * @return Puntero a spi_t si existe, NULL si índice inválido.
 */
spi_t *spi_get_by_num(size_t num);

#endif // HW_CONFIG_H
