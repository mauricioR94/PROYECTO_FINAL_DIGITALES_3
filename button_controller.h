/**
 * @file button_controller.h
 * @brief Controlador de los botones de notas musicales.
 * 
 * Gestiona:
 *  - Lectura de 7 botones correspondientes a notas musicales.
 *  - Debounce individual.
 *  - Asociación GPIO -> nota -> archivo WAV.
 *  - Detección de eventos y entrega del índice de nota.
 * 
 * Este módulo trabaja en conjunto con el reproductor WAV.
 * 
 * @authors
 *  - Mauricio Reyes Rosero
 *  - Reinaldo Marín Nieto
 *  - Daniel Pérez Gallego
 *  - Jorge Arroyo Niño
 */

#ifndef BUTTON_CONTROLLER_H
#define BUTTON_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

/** GPIO nota DO. */
#define BTN_DO   18
/** GPIO nota RE. */
#define BTN_RE   20
/** GPIO nota MI. */
#define BTN_MI   19
/** GPIO nota FA. */
#define BTN_FA    9
/** GPIO nota SOL. */
#define BTN_SOL   8
/** GPIO nota LA. */
#define BTN_LA    7
/** GPIO nota SI. */
#define BTN_SI    6

/**
 * @brief Estructura que define un botón musical.
 */
typedef struct {
    uint32_t    gpio;        /**< GPIO asociado al botón. */
    const char *note_name;   /**< Nombre de la nota musical. */
    const char *wav_file;    /**< Ruta base del archivo WAV. */
    bool        was_pressed; /**< Estado previo para debounce. */
} button_t;

/**
 * @brief Inicializa los GPIO usados para los botones de notas.
 */
void button_controller_init(void);

/**
 * @brief Procesa los botones y detecta si alguno fue presionado.
 * 
 * @return Índice 0..6 del botón presionado o -1 si no hubo evento.
 */
int button_controller_process(void);

/**
 * @brief Obtiene el nombre de la nota asociada a un índice.
 * @param index Índice del botón (0..6).
 * @return Cadena con el nombre de la nota o NULL si es inválido.
 */
const char* button_controller_get_note_name(int index);

/**
 * @brief Obtiene el nombre del archivo WAV asociado a la nota.
 * @param index Índice del botón (0..6).
 * @return Ruta del archivo WAV o NULL si es inválido.
 */
const char* button_controller_get_wav_file(int index);

#endif
