#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/time.h"      // repeating_timer_t, add_repeating_timer_ms
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"

#include "i2s_tx.pio.h"

// Pines del DAC UDA1334A (CJMCU-1334)
#define PIN_BCLK   10
#define PIN_LRCK   11
#define PIN_DIN    12

// Pines de botones (un extremo a GND, el otro al GPIO con pull-up)
#define BTN_DO   18
#define BTN_RE   20
#define BTN_MI   19
#define BTN_FA    9
#define BTN_SOL   8
#define BTN_LA    7
#define BTN_SI    6

// Frecuencia de muestreo de audio (aprox)
#define SAMPLE_RATE 22050.0f

// Escala simple (Do4 a Do5 aprox)
#define NUM_NOTES 8
static const float note_freqs[NUM_NOTES] = {
    261.63f, // 0 - Do (C4)
    293.66f, // 1 - Re (D4)
    329.63f, // 2 - Mi (E4)
    349.23f, // 3 - Fa (F4)
    392.00f, // 4 - Sol (G4)
    440.00f, // 5 - La (A4)
    493.88f, // 6 - Si (B4)
    523.25f  // 7 - Do (C5) - no lo usamos aquí, pero lo dejamos
};

// Nota actual (-1 = silencio)
volatile int current_note_index = -1;

// --- Interrupción: timer que hace polling de botones --- //
bool button_timer_callback(repeating_timer_t *rt) {
    (void)rt; // evitar warning

    int new_index = -1;

    // Botones activos en bajo (un extremo a GND, pull-up interno)
    if (!gpio_get(BTN_DO)) {
        new_index = 0;      // Do
    } else if (!gpio_get(BTN_RE)) {
        new_index = 1;      // Re
    } else if (!gpio_get(BTN_MI)) {
        new_index = 2;      // Mi
    } else if (!gpio_get(BTN_FA)) {
        new_index = 3;      // Fa
    } else if (!gpio_get(BTN_SOL)) {
        new_index = 4;      // Sol
    } else if (!gpio_get(BTN_LA)) {
        new_index = 5;      // La
    } else if (!gpio_get(BTN_SI)) {
        new_index = 6;      // Si
    } else {
        new_index = -1;     // Ningún botón -> silencio
    }

    current_note_index = new_index;
    return true; // seguir llamando
}

int main() {
    stdio_init_all();

    // --- Inicialización de botones --- //
    const uint button_pins[] = { BTN_DO, BTN_RE, BTN_MI, BTN_FA, BTN_SOL, BTN_LA, BTN_SI };
    for (int i = 0; i < 7; i++) {
        gpio_init(button_pins[i]);
        gpio_set_dir(button_pins[i], GPIO_IN);
        gpio_pull_up(button_pins[i]);  // entradas con pull-up; activo en 0
    }

    // --- Inicialización PIO I2S (igual que antes) --- //
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);

    uint offset = pio_add_program(pio, &i2s_tx_program);

    // BCLK = PIN_BCLK, LRCK = PIN_BCLK + 1 = PIN_LRCK
    i2s_tx_program_init(pio, sm, offset, PIN_DIN, PIN_BCLK);

    uint32_t sys_clk = clock_get_hz(clk_sys);
    float pio_clk = SAMPLE_RATE * 32.0f * 2.0f;
    float div = (float)sys_clk / pio_clk;
    pio_sm_set_clkdiv(pio, sm, div);

    pio_sm_set_enabled(pio, sm, true);

    // --- Timer periódico para leer botones (interrupciones) --- //
    repeating_timer_t button_timer;
    // Cada 5 ms aprox: suficiente para detectar pulsos humanos y hacer "debounce" básico
    add_repeating_timer_ms(5, button_timer_callback, NULL, &button_timer);

    // --- Generador de onda cuadrada por DDS simple --- //
    float phase = 0.0f;  // rango [0,1)
    const float amplitude = 30000.0f;

    while (true) {
        // Polling: esperar hueco en el FIFO de TX de la PIO
        if (!pio_sm_is_tx_fifo_full(pio, sm)) {
            int note = current_note_index;
            int16_t sample;

            if (note >= 0 && note <= 6) {
                float freq = note_freqs[note];
                float phase_inc = freq / SAMPLE_RATE; // ciclos por muestra

                phase += phase_inc;
                if (phase >= 1.0f) {
                    phase -= 1.0f;
                }

                // Onda cuadrada: positivo la mitad del ciclo, negativo la otra mitad
                sample = (phase < 0.5f) ? (int16_t)amplitude : (int16_t)(-amplitude);
            } else {
                // Sin nota: silencio
                sample = 0;
            }

            // Mismo valor en L y R (estéreo "mono")
            uint32_t frame = ((uint16_t)sample << 16) | ((uint16_t)sample);

            // Enviar frame al PIO (32 bits: 16 L + 16 R)
            pio_sm_put(pio, sm, frame);
        }
    }

    return 0;
}
