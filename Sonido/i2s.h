#ifndef I2S_H
#define I2S_H

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "i2s.pio.h"

#define PIN_BCLK   18
#define PIN_LRCLK  19
#define PIN_SD     20

static inline void i2s_init(PIO pio, uint sm, uint sample_rate) {
    uint offset = pio_add_program(pio, &i2s_tx_program);

    pio_gpio_init(pio, PIN_LRCLK);
    pio_gpio_init(pio, PIN_SD);
    pio_gpio_init(pio, PIN_BCLK);

    pio_sm_set_consecutive_pindirs(pio, sm, PIN_LRCLK, 2, true);
    pio_sm_set_consecutive_pindirs(pio, sm, PIN_BCLK, 1, true);

    pio_sm_config c = i2s_tx_program_get_default_config(offset);

    sm_config_set_out_pins(&c, PIN_SD, 1);
    sm_config_set_sideset_pins(&c, PIN_BCLK);

    sm_config_set_out_shift(&c, false, true, 32);

    float div = (float)clock_get_hz(clk_sys) / (sample_rate * 64.0f);
    sm_config_set_clkdiv(&c, div);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

#endif
