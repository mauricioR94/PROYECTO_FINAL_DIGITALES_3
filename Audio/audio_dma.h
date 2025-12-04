#ifndef AUDIO_DMA_H
#define AUDIO_DMA_H

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

// Tamaño del buffer (frames stereo)
#define AUDIO_FRAMES 256
#define AUDIO_BYTES (AUDIO_FRAMES * 4)  // 4 bytes por frame (L y R en 16 bits)

static uint dma_chan;
static volatile bool dma_finished = false;

static uint8_t audio_buf0[AUDIO_BYTES];
static uint8_t audio_buf1[AUDIO_BYTES];
static uint8_t *active_buf = audio_buf0;
static uint8_t *next_buf   = audio_buf1;

static void __isr dma_handler() {
    dma_hw->ints0 = 1u << dma_chan;   // limpiar IRQ
    dma_finished = true;
}

static inline void audio_dma_init(PIO pio, uint sm) {
    dma_chan = dma_claim_unused_channel(true);

    dma_channel_config cfg = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&cfg, true);
    channel_config_set_write_increment(&cfg, false);
    channel_config_set_dreq(&cfg, pio_get_dreq(pio, sm, true));

    dma_channel_configure(
        dma_chan,
        &cfg,
        &pio->txf[sm],     // destino: FIFO TX del PIO
        active_buf,        // origen
        AUDIO_BYTES,
        false
    );

    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);
    dma_channel_set_irq0_enabled(dma_chan, true);

    dma_channel_start(dma_chan);
}

static inline void audio_dma_swap_buffers() {
    uint8_t *tmp = active_buf;
    active_buf = next_buf;
    next_buf = tmp;

    dma_channel_set_read_addr(dma_chan, active_buf, true);
}

#endif
