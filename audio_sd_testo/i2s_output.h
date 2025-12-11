#ifndef I2S_OUTPUT_H
#define I2S_OUTPUT_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/pio.h"

// Inicializar salida I2S con PIO
bool i2s_output_init(uint sample_rate);

// Enviar un frame de audio (32 bits: 16L + 16R)
void i2s_output_send_frame(int16_t left, int16_t right);

// Verificar si hay espacio en el FIFO
bool i2s_output_can_send();

// Detener I2S
void i2s_output_stop();

// Obtener información del estado
typedef struct {
    PIO pio;
    uint sm;
    bool active;
    uint32_t sample_rate;
} i2s_info_t;

i2s_info_t i2s_output_get_info();

#endif // I2S_OUTPUT_H