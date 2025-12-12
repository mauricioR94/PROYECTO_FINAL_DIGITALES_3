/**
 * @file i2s_output.h
 * @brief Interfaz de salida I2S basada en PIO.
 * 
 * Permite:
 *  - Inicializar transmisión I2S con frecuencia de muestreo variable.
 *  - Enviar frames estéreo de 32 bits (16L + 16R).
 *  - Consultar si el FIFO del PIO tiene espacio.
 *  - Detener la transmisión.
 */

#ifndef I2S_OUTPUT_H
#define I2S_OUTPUT_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/pio.h"

/**
 * @brief Inicializa salida I2S con un sample rate dado.
 * @param sample_rate Frecuencia de muestreo (ej. 44100 o 48000).
 * @return true si se inicializó correctamente.
 */
bool i2s_output_init(uint sample_rate);

/**
 * @brief Envía un frame I2S (bloqueante si FIFO está lleno).
 */
void i2s_output_send_frame(int16_t left, int16_t right);

/**
 * @brief Indica si hay espacio en el FIFO TX del PIO.
 */
bool i2s_output_can_send();

/**
 * @brief Detiene la salida I2S.
 */
void i2s_output_stop();

/**
 * @brief Información del estado actual del módulo I2S.
 */
typedef struct {
    PIO pio;               /**< Instancia PIO usada. */
    uint sm;               /**< State machine asignada. */
    bool active;           /**< true si está transmitiendo. */
    uint32_t sample_rate;  /**< Frecuencia actual. */
} i2s_info_t;

/**
 * @brief Devuelve los datos actuales del periférico I2S.
 */
i2s_info_t i2s_output_get_info();

#endif
