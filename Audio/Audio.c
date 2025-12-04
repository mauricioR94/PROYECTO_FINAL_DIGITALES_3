#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "i2s.h"
#include "i2s.pio.h"

#define PIN_BCLK 10
#define PIN_LRCK 11
#define PIN_DIN  12

#define SAMPLE_RATE     44100
#define FRECUENCIA_TONO 440.0f
#define BUFFER_SIZE     256

i2s_inst_t i2s;

uint32_t bufferA[BUFFER_SIZE];
uint32_t bufferB[BUFFER_SIZE];

void fill_sine(uint32_t *buffer) {
    static float fase = 0.0f;
    float step = 2.0f * M_PI * FRECUENCIA_TONO / SAMPLE_RATE;

    for (int i = 0; i < BUFFER_SIZE; i++) {
        float v = sinf(fase);
        fase += step;
        if (fase > 2*M_PI) fase -= 2*M_PI;

        int16_t sample = (int16_t)(v * 30000);

        uint32_t stereo = ((uint32_t)sample << 16) | (uint16_t)sample;
        buffer[i] = stereo;
    }
}

int main() {
    stdio_init_all();

    i2s_init(&i2s,
             pio0,
             0,
             PIN_DIN,
             PIN_BCLK,
             PIN_LRCK,
             SAMPLE_RATE);

    fill_sine(bufferA);
    fill_sine(bufferB);

    i2s_start_dma(&i2s, bufferA, bufferB, BUFFER_SIZE);

    while (1) {
        sleep_ms(10);
    }
}
