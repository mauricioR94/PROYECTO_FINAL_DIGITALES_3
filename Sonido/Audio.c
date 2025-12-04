#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "i2s.h"
#include "audio_dma.h"

#define SAMPLE_RATE 44100
#define TONE_FREQ   440.0f   // tono de prueba

void fill_buffer(uint8_t *buf) {
    static float phase = 0.0f;
    float step = 2 * M_PI * TONE_FREQ / SAMPLE_RATE;

    for (int i = 0; i < AUDIO_FRAMES; i++) {
        float s = sinf(phase);
        phase += step;
        if (phase > 2*M_PI) phase -= 2*M_PI;

        int16_t sample = (int16_t)(s * 30000);

        int32_t frame = ((int32_t)sample << 16) | (uint16_t)sample;

        ((uint32_t*)buf)[i] = frame;
    }
}

int main() {
    stdio_init_all();

    PIO pio = pio0;
    uint sm = 0;

    i2s_init(pio, sm, SAMPLE_RATE);

    fill_buffer(active_buf);
    fill_buffer(next_buf);

    audio_dma_init(pio, sm);

    while (true) {
        if (dma_finished) {
            dma_finished = false;

            fill_buffer(next_buf);

            audio_dma_swap_buffers();
        }
    }
}
