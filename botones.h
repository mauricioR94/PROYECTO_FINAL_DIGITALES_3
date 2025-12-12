/**
 * @file botones.h
 * @brief Módulo de manejo de los botones principales del sistema (slot, siguiente y anterior).
 * 
 * Gestiona:
 *  - Lectura de 3 botones conectados a GPIO.
 *  - Debounce por software.
 *  - Generación de eventos BTN_*.
 * 
 * @authors
 *  - Mauricio Reyes Rosero
 *  - Reinaldo Marín Nieto
 *  - Daniel Pérez Gallego
 *  - Jorge Arroyo Niño
 */

#ifndef BOTONES_H
#define BOTONES_H

#include <stdint.h>
#include <stdbool.h>

/** GPIO del botón de selección de slot. */
#define BTN_SLOT 14

/** GPIO del botón para pasar al siguiente instrumento. */
#define BTN_NEXT 13

/** GPIO del botón para ir al instrumento anterior. */
#define BTN_PREV 15

/**
 * @brief Eventos que pueden generar los botones.
 */
typedef enum {
    BTN_NONE = 0,     /**< No hubo evento. */
    BTN_SLOT_EVT,     /**< Botón SLOT presionado. */
    BTN_NEXT_EVT,     /**< Botón NEXT presionado. */
    BTN_PREV_EVT      /**< Botón PREV presionado. */
} boton_evento_t;

/**
 * @brief Inicializa los GPIO asociados a los botones.
 */
void botones_init(void);

/**
 * @brief Actualiza el estado de los botones aplicando debounce.
 * 
 * Debe llamarse periódicamente.
 */
void botones_update(void);

/**
 * @brief Obtiene el último evento detectado y lo limpia.
 * @return Evento BTN_* o BTN_NONE.
 */
boton_evento_t botones_get_evento(void);

#endif
