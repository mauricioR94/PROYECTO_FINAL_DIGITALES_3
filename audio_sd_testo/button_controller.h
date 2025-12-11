#ifndef BUTTON_CONTROLLER_H
#define BUTTON_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

// ========================================
// PINES DE BOTONES (igual que Proyecto 2)
// ========================================
#define BTN_DO   18  // Do (C4)
#define BTN_RE   20  // Re (D4)
#define BTN_MI   19  // Mi (E4)
#define BTN_FA    9  // Fa (F4)
#define BTN_SOL   8  // Sol (G4)
#define BTN_LA    7  // La (A4)
#define BTN_SI    6  // Si (B4)

// Estructura de botón
typedef struct {
    uint8_t gpio;
    const char *note_name;
    const char *wav_file;
    bool was_pressed;
} button_t;

// Inicializar sistema de botones
void button_controller_init();

// Procesar botones (llamar en loop principal)
// Retorna el índice del botón presionado (0-6) o -1 si ninguno
int button_controller_process();

// Obtener nombre de la nota
const char* button_controller_get_note_name(int index);

// Obtener ruta del archivo WAV
const char* button_controller_get_wav_file(int index);

#endif // BUTTON_CONTROLLER_H