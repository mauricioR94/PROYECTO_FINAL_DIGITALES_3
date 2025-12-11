#include "button_controller.h"
#include "pico/stdlib.h"
#include <stdio.h>

// Configuración de botones y archivos
// El orden del arreglo define el índice que devuelve button_controller_process():
//  0 -> Do, 1 -> Re, 2 -> Mi, 3 -> Fa, 4 -> Sol, 5 -> La, 6 -> Si
static button_t buttons[7] = {
    {BTN_DO,  "Do",  "0:/do.wav",  false},
    {BTN_RE,  "Re",  "0:/re.wav",  false},
    {BTN_MI,  "Mi",  "0:/mi.wav",  false},
    {BTN_FA,  "Fa",  "0:/fa.wav",  false},
    {BTN_SOL, "Sol", "0:/sol.wav", false},
    {BTN_LA,  "La",  "0:/la.wav",  false},
    {BTN_SI,  "Si",  "0:/si.wav",  false}
};

static uint32_t last_press_time = 0;

void button_controller_init() {
    printf("Inicializando controlador de botones.\n");

    // Configurar todos los botones como entradas con pull-up
    for (int i = 0; i < 7; i++) {
        gpio_init(buttons[i].gpio);
        gpio_set_dir(buttons[i].gpio, GPIO_IN);
        gpio_pull_up(buttons[i].gpio);

        printf("  Botón %s (GPIO %d) -> %s\n",
               buttons[i].note_name,
               buttons[i].gpio,
               buttons[i].wav_file);
    }

    printf("✅ Botones inicializados (activo en bajo)\n\n");
}

int button_controller_process() {
    // Debounce más rápido para que el instrumento sea más “tocable”
    const uint32_t debounce_delay = 80; // ms

    uint32_t now = to_ms_since_boot(get_absolute_time());

    // No aceptar nuevas pulsaciones demasiado rápido
    if ((now - last_press_time) < debounce_delay) {
        return -1;
    }

    // Escanear botones (prioridad: el primero que se detecte)
    for (int i = 0; i < 7; i++) {
        bool is_pressed = !gpio_get(buttons[i].gpio); // Activo en bajo

        // Flanco de bajada: pasó de NO presionado a presionado
        if (is_pressed && !buttons[i].was_pressed) {
            buttons[i].was_pressed = true;
            last_press_time = now;

            printf("🎹 Botón presionado: %s (GPIO %d, índice %d)\n",
                   buttons[i].note_name, buttons[i].gpio, i);
            return i;
        }

        // Si se soltó, reseteamos el estado
        if (!is_pressed) {
            buttons[i].was_pressed = false;
        }
    }

    return -1; // Ningún botón nuevo presionado
}

const char* button_controller_get_note_name(int index) {
    if (index >= 0 && index < 7) {
        return buttons[index].note_name;
    }
    return "Unknown";
}

const char* button_controller_get_wav_file(int index) {
    if (index >= 0 && index < 7) {
        return buttons[index].wav_file;
    }
    return NULL;
}
