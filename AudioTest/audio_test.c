#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "hardware/timer.h"

#include "i2s_tx.pio.h"

// Pines del DAC UDA1334A (CJMCU-1334)
#define PIN_BCLK   10
#define PIN_LRCK   11
#define PIN_DIN    12

// Frecuencia de muestreo de audio (aprox)
#define SAMPLE_RATE 22050.0f

// Escala simple (Do4 a Do5 aprox)
#define NUM_NOTES 8
static const float note_freqs[NUM_NOTES] = {
    261.63f, // C4
    293.66f, // D4
    329.63f, // E4
    349.23f, // F4
    392.00f, // G4
    440.00f, // A4
    493.88f, // B4
    523.25f  // C5
};

// Nota actual (cambiada por interrupción de timer)
volatile int current_note_index = 0;

// Callback de timer: cambia de nota cada 500 ms
bool note_timer_callback(repeating_timer_t *rt) {
    current_note_index = (current_note_index + 1) % NUM_NOTES;
    return true; // seguir llamando
}

int main() {
    stdio_init_all();

    // Seleccionamos PIO0 y una state machine libre
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);

    // Cargar el programa de PIO
    uint offset = pio_add_program(pio, &i2s_tx_program);

    // Inicializar programa I2S en PIO
    // BCLK = PIN_BCLK, LRCK = PIN_BCLK + 1 = PIN_LRCK
    i2s_tx_program_init(pio, sm, offset, PIN_DIN, PIN_BCLK);

    // Calcular divisor de reloj para la PIO:
    //
    // La PIO ejecuta 2 instrucciones por bit (out + jmp)
    // Bits por frame = 32 (16 L + 16 R)
    //
    // BCLK = SAMPLE_RATE * 32
    // Fpio = BCLK * 2 (porque 2 instrucciones por bit)
    //
    // div = Fsys / Fpio
    uint32_t sys_clk = clock_get_hz(clk_sys);
    float pio_clk = SAMPLE_RATE * 32.0f * 2.0f;
    float div = (float)sys_clk / pio_clk;
    pio_sm_set_clkdiv(pio, sm, div);

    // Habilitar la state machine
    pio_sm_set_enabled(pio, sm, true);

    // Timer para cambiar la nota (interrupciones)
    repeating_timer_t note_timer;
    add_repeating_timer_ms(500, note_timer_callback, NULL, &note_timer);

    // Generador de onda cuadrada por DDS simple
    float phase = 0.0f;  // rango [0,1)
    const float amplitude = 30000.0f;

    while (true) {
        // Polling: esperar hueco en el FIFO de TX de la PIO
        if (!pio_sm_is_tx_fifo_full(pio, sm)) {
            float freq = note_freqs[current_note_index];
            float phase_inc = freq / SAMPLE_RATE; // ciclos por muestra

            phase += phase_inc;
            if (phase >= 1.0f) {
                phase -= 1.0f;
            }

            // Onda cuadrada: positivo la mitad del ciclo, negativo la otra mitad
            int16_t sample = (phase < 0.5f) ? (int16_t)amplitude : (int16_t)(-amplitude);

            // Mismo valor en L y R (estéreo "mono")
            uint32_t frame = ((uint16_t)sample << 16) | ((uint16_t)sample);

            // Enviar frame al PIO (32 bits: 16 L + 16 R)
            pio_sm_put(pio, sm, frame);
        }

        // No dormir: queremos mantener el buffer lleno
        // (si quieres ver debug por USB, podrías añadir prints esporádicos,
        //  pero no dentro de este bucle o romperás el timing)
    }

    return 0;
}
