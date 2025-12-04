#include "i2s.h"
#include "i2s.pio.h"
#include "hardware/clocks.h"

static uint dma_chan;
static const uint32_t *bufA;
static const uint32_t *bufB;
static size_t buf_size;

static void dma_handler() {
    dma_hw->ints0 = 1u << dma_chan;

    static bool toggle = false;

    dma_channel_set_read_addr(dma_chan,
                              toggle ? bufA : bufB,
                              true);
    toggle = !toggle;
}

void i2s_init(i2s_inst_t *i2s,
              PIO pio,
              uint sm,
              uint pin_data,
              uint pin_bclk,
              uint pin_lrclk,
              uint sample_rate) {

    i2s->pio = pio;
    i2s->sm = sm;

    uint offset = pio_add_program(pio, &i2s_tx_program);

    pio_gpio_init(pio, pin_data);
    pio_gpio_init(pio, pin_bclk);
    pio_gpio_init(pio, pin_lrclk);

    pio_sm_set_consecutive_pindirs(pio, sm, pin_data, 1, true);
    pio_sm_set_consecutive_pindirs(pio, sm, pin_bclk, 1, true);
    pio_sm_set_consecutive_pindirs(pio, sm, pin_lrclk, 1, true);

    float clk_div = (float)clock_get_hz(clk_sys) / (sample_rate * 64);

    i2s_tx_program_init(pio, sm, offset, pin_data, pin_bclk, pin_lrclk);
    pio_sm_set_clkdiv(pio, sm, clk_div);

    pio_sm_set_enabled(pio, sm, true);
}

void i2s_start_dma(i2s_inst_t *i2s,
                   const uint32_t *bufferA,
                   const uint32_t *bufferB,
                   size_t samples_per_buffer) {

    bufA = bufferA;
    bufB = bufferB;
    buf_size = samples_per_buffer;

    dma_chan = dma_claim_unused_channel(true);

    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pio_get_dreq(i2s->pio, i2s->sm, true));
    channel_config_set_irq_quiet(&c, false);

    dma_channel_configure(
        dma_chan,
        &c,
        &i2s->pio->txf[i2s->sm],
        bufA,
        samples_per_buffer,
        false
    );

    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_channel_start(dma_chan);
}
