/**
 * @file sd_manager.h
 * @brief Módulo de gestión de tarjeta SD usando FATFS y DMA.
 *
 * Proporciona funciones para inicializar, desmontar, verificar estado
 * y listar archivos WAV en la tarjeta SD.
 */

#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <stdbool.h>
#include "ff.h"

/**
 * @brief Inicializa la tarjeta SD y monta el sistema de archivos.
 *
 * Configura la interfaz SPI (según hw_config), monta FATFS
 * y habilita el uso de archivos.
 *
 * @return true si la SD fue montada correctamente.
 */
bool sd_manager_init();

/**
 * @brief Desmonta la tarjeta SD si está activa.
 *
 * Libera recursos y marca el módulo como no disponible.
 */
void sd_manager_deinit();

/**
 * @brief Indica si la SD está montada y lista para operar.
 *
 * @return true si la SD está disponible.
 */
bool sd_manager_is_ready();

/**
 * @brief Recorre el directorio raíz e imprime todos los archivos .wav.
 *
 * Solo funciona si la SD está montada. Muestra nombre y tamaño.
 */
void sd_manager_list_wav_files();

#endif // SD_MANAGER_H
