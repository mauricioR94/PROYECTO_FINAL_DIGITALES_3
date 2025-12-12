/**
 * @file input_buttons.c
 * @brief Manejo de botones de notas musicales y botones del selector de instrumentos.
 *
 * Este módulo gestiona interrupciones GPIO, debounce y generación de eventos
 * tanto para los botones de notas (Do..Si) como para los botones de control
 * (SLOT, NEXT, PREV).
 */

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "button_controller.h"
#include "botones.h"

/** Tabla de botones de notas musicales. */
static button_t buttons[7] = {
    {BTN_DO,  "Do",  "0:/do.wav",  false},
    {BTN_RE,  "Re",  "0:/re.wav",  false},
    {BTN_MI,  "Mi",  "0:/mi.wav",  false},
    {BTN_FA,  "Fa",  "0:/fa.wav",  false},
    {BTN_SOL, "Sol", "0:/sol.wav", false},
    {BTN_LA,  "La",  "0:/la.wav",  false},
    {BTN_SI,  "Si",  "0:/si.wav",  false}
};

/** Flags de interrupción para botones de nota. */
static volatile bool note_irq_flags[7] = {0};
/** Timestamps para debounce de notas. */
static uint32_t last_event_time_notas[7] = {0};

/** Flags de interrupción para botones del selector. */
static volatile bool selector_irq_flags[3] = {0};
/** Timestamps para debounce del selector. */
static uint32_t last_event_time_sel[3] = {0};

/** Último evento generado por botones del selector. */
static boton_evento_t ultimo_evento = BTN_NONE;

/**
 * @brief Convierte un GPIO a su índice dentro del arreglo de notas.
 * @param gpio GPIO activado.
 * @return Índice 0..6 o -1 si no corresponde.
 */
static int note_index_from_gpio(uint gpio) {
    for (int i = 0; i < 7; i++) {
        if (buttons[i].gpio == gpio) return i;
    }
    return -1;
}

/**
 * @brief Convierte GPIO a índice de selector (SLOT, NEXT, PREV).
 * @param gpio GPIO activado.
 * @return Índice 0..2 o -1 si no aplica.
 */
static int selector_index_from_gpio(uint gpio) {
    if (gpio == BTN_SLOT) return 0;
    if (gpio == BTN_NEXT) return 1;
    if (gpio == BTN_PREV) return 2;
    return -1;
}

/**
 * @brief Callback de interrupción común para todos los botones.
 * Detecta flanco de bajada y establece flags correspondientes.
 */
static void gpio_irq_handler(uint gpio, uint32_t events) {
    if (!(events & GPIO_IRQ_EDGE_FALL)) return;

    int n = note_index_from_gpio(gpio);
    if (n >= 0) {
        note_irq_flags[n] = true;
        return;
    }

    int s = selector_index_from_gpio(gpio);
    if (s >= 0) {
        selector_irq_flags[s] = true;
        return;
    }
}

/**
 * @brief Inicializa los botones de notas y sus interrupciones.
 */
void button_controller_init(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    for (int i = 0; i < 7; i++) {
        gpio_init(buttons[i].gpio);
        gpio_set_dir(buttons[i].gpio, GPIO_IN);
        gpio_pull_up(buttons[i].gpio);

        note_irq_flags[i] = false;
        last_event_time_notas[i] = now;

        gpio_set_irq_enabled_with_callback(
            buttons[i].gpio,
            GPIO_IRQ_EDGE_FALL,
            true,
            gpio_irq_handler
        );
    }
}

/**
 * @brief Procesa las notas, aplicando debounce y devolviendo la nota presionada.
 * @return Índice 0..6 si hay nota válida, -1 si no.
 */
int button_controller_process(void) {
    const uint32_t DEBOUNCE_MS = 120;
    uint32_t now = to_ms_since_boot(get_absolute_time());

    for (int i = 0; i < 7; i++) {
        if (note_irq_flags[i]) {
            note_irq_flags[i] = false;

            if (now - last_event_time_notas[i] >= DEBOUNCE_MS) {
                last_event_time_notas[i] = now;
                return i;
            }
        }
    }
    return -1;
}

/**
 * @brief Obtiene el nombre de una nota según su índice.
 * @param index Índice 0..6.
 * @return Cadena con nombre o "Unknown".
 */
const char* button_controller_get_note_name(int index) {
    if (index >= 0 && index < 7) return buttons[index].note_name;
    return "Unknown";
}

/**
 * @brief Obtiene el archivo WAV asociado a una nota.
 * @param index Índice 0..6.
 * @return Ruta del wav o NULL.
 */
const char* button_controller_get_wav_file(int index) {
    if (index >= 0 && index < 7) return buttons[index].wav_file;
    return 0;
}

/**
 * @brief Inicializa los botones del selector (SLOT, NEXT, PREV).
 */
void botones_init(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    uint selector_pins[3] = { BTN_SLOT, BTN_NEXT, BTN_PREV };

    for (int i = 0; i < 3; i++) {
        uint gpio = selector_pins[i];

        gpio_init(gpio);
        gpio_set_dir(gpio, GPIO_IN);
        gpio_pull_up(gpio);

        selector_irq_flags[i] = false;
        last_event_time_sel[i] = now;

        gpio_set_irq_enabled_with_callback(
            gpio,
            GPIO_IRQ_EDGE_FALL,
            true,
            gpio_irq_handler
        );
    }

    ultimo_evento = BTN_NONE;
}

/**
 * @brief Procesa debounce y genera eventos de selector.
 */
void botones_update(void) {
    const uint32_t DEBOUNCE_MS = 120;
    uint32_t now = to_ms_since_boot(get_absolute_time());

    for (int i = 0; i < 3; i++) {
        if (selector_irq_flags[i]) {
            selector_irq_flags[i] = false;

            if (now - last_event_time_sel[i] >= DEBOUNCE_MS) {
                last_event_time_sel[i] = now;

                if (i == 0) ultimo_evento = BTN_SLOT_EVT;
                else if (i == 1) ultimo_evento = BTN_NEXT_EVT;
                else ultimo_evento = BTN_PREV_EVT;
            }
        }
    }
}

/**
 * @brief Obtiene el último evento de selector y lo limpia.
 * @return Evento BTN_SLOT_EVT, BTN_NEXT_EVT, BTN_PREV_EVT o BTN_NONE.
 */
boton_evento_t botones_get_evento(void) {
    boton_evento_t e = ultimo_evento;
    ultimo_evento = BTN_NONE;
    return e;
}
