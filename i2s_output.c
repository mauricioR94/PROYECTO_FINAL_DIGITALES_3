/**
 * @file i2s_output.c
 * @brief Implementación de salida I2S usando PIO.
 */

#include "i2s_output.h"
#include "hw_config.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include "i2s_tx.pio.h"

static PIO  i2s_pio         = NULL;
static uint i2s_sm          = 0;
static uint i2s_offset      = 0;
static bool i2s_initialized = false;
static bool i2s_active      = false;
static uint32_t current_sample_rate = 0;

// 96 ciclos PIO por frame estéreo en i2s_tx.pio
#define I2S_PIO_CYCLES_PER_FRAME 96.0f

bool i2s_output_init(uint sample_rate) {
    printf("Inicializando salida I2S.\n");

    if (i2s_initialized) {
        pio_sm_set_enabled(i2s_pio, i2s_sm, false);
        pio_sm_clear_fifos(i2s_pio, i2s_sm);

        uint32_t sys_clk = clock_get_hz(clk_sys);
        float pio_clk = (float)sample_rate * I2S_PIO_CYCLES_PER_FRAME;
        float div = (float)sys_clk / pio_clk;

        pio_sm_config c = i2s_tx_program_get_default_config(i2s_offset);
        sm_config_set_out_pins(&c, I2S_DIN_PIN, 1);
        sm_config_set_sideset_pins(&c, I2S_BCLK_PIN);
        sm_config_set_out_shift(&c, false, true, 32);
        sm_config_set_clkdiv(&c, div);

        pio_sm_init(i2s_pio, i2s_sm, i2s_offset, &c);
        pio_sm_set_enabled(i2s_pio, i2s_sm, true);

        i2s_active = true;
        current_sample_rate = sample_rate;

        printf("  I2S reconfigurado:\n");
        printf("   PIO: pio%d, SM: %u\n", pio_get_index(i2s_pio), i2s_sm);
        printf("   Sample Rate: %lu Hz\n", sample_rate);
        printf("   Divider: %.4f\n", div);

        return true;
    }

    i2s_pio = pio0;

    if (!pio_can_add_program(i2s_pio, &i2s_tx_program)) {
        printf("Error: no hay espacio para programa PIO\n");
        return false;
    }

    i2s_sm = pio_claim_unused_sm(i2s_pio, true);
    i2s_offset = pio_add_program(i2s_pio, &i2s_tx_program);

    i2s_tx_program_init(i2s_pio, i2s_sm, i2s_offset, I2S_DIN_PIN, I2S_BCLK_PIN);

    uint32_t sys_clk = clock_get_hz(clk_sys);
    float pio_clk = (float)sample_rate * I2S_PIO_CYCLES_PER_FRAME;
    float div = (float)sys_clk / pio_clk;

    pio_sm_set_clkdiv(i2s_pio, i2s_sm, div);
    pio_sm_set_enabled(i2s_pio, i2s_sm, true);

    i2s_active = true;
    i2s_initialized = true;
    current_sample_rate = sample_rate;

    printf(" I2S inicializado:\n");
    printf("   PIO: pio%d, SM: %u\n", pio_get_index(i2s_pio), i2s_sm);
    printf("   Sample Rate: %lu Hz\n", sample_rate);
    printf("   Divider: %.4f\n", div);

    return true;
}

void i2s_output_send_frame(int16_t left, int16_t right) {
    if (!i2s_active) return;

    uint32_t frame = ((uint32_t)(uint16_t)left << 16)
                     | (uint16_t)right;

    pio_sm_put_blocking(i2s_pio, i2s_sm, frame);
}

bool i2s_output_can_send() {
    return i2s_active && !pio_sm_is_tx_fifo_full(i2s_pio, i2s_sm);
}

void i2s_output_stop() {
    if (i2s_active) {
        pio_sm_set_enabled(i2s_pio, i2s_sm, false);
        pio_sm_clear_fifos(i2s_pio, i2s_sm);
        i2s_active = false;
        printf("I2S detenido\n");
    }
}

i2s_info_t i2s_output_get_info() {
    i2s_info_t info = {
        .pio = i2s_pio,
        .sm = i2s_sm,
        .active = i2s_active,
        .sample_rate = current_sample_rate
    };
    return info;
}
