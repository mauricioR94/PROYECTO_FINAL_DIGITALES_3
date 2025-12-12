/**
 * @file instrument_ui.c
 * @brief Implementación de carga y manejo de instrumentos y parte del sistema de slots.
 *
 * Implementa:
 *  - Lectura de 0:/index.txt para poblar la tabla de instrumentos.
 *  - Función de debug para volcar la tabla por consola.
 *  - Inicialización y actualización simple del sistema de slots.
 *
 * Nota: las estructuras y constantes relacionadas con "sistema" (SLOT_H, SLOT_V, lcd_mostrar_estado)
 * se asumen definidas en otros módulos (sistema.h, lcd.h).
 *
 * @authors
 *  - Mauricio Reyes Rosero
 *  - Reinaldo Marín Nieto
 *  - Daniel Pérez Gallego
 *  - Jorge Arroyo Niño
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "ff.h"

#include "instrumentos.h"
#include "sistema.h"
#include "lcd.h"
#include "botones.h"


char    instrumentos[MAX_INSTRUMENTOS][MAX_NOMBRE_INSTR + 1];
uint8_t instrumentos_id[MAX_INSTRUMENTOS];
uint8_t total_instrumentos = 0;

/**
 * @brief Carga los instrumentos desde el archivo "0:/index.txt".
 *
 * El formato esperado por línea es: i<id>-<nombre>
 *
 * - Ignora líneas que no comiencen con 'i'.
 * - Extrae el id numérico y el nombre hasta el primer CR/LF.
 * - Copia el nombre truncándolo a MAX_NOMBRE_INSTR.
 *
 * Si ocurre algún error al abrir o leer, la función retorna sin modificar la tabla.
 */
void cargar_instrumentos_desde_index(void) {
    FIL file;
    FRESULT fr;

    total_instrumentos = 0;

    fr = f_open(&file, "0:/index.txt", FA_READ);
    if (fr != FR_OK) {
        printf("No se pudo abrir index.txt (FR=%d)\n", fr);
        return;
    }

    char line[64];

    while (total_instrumentos < MAX_INSTRUMENTOS &&
           f_gets(line, sizeof(line), &file) != NULL) {

        /* Eliminar CR/LF final */
        char *p = line;
        while (*p && *p != '\r' && *p != '\n') {
            p++;
        }
        *p = '\0';

        /* Filtrar líneas no válidas */
        if (line[0] != 'i') {
            continue;
        }

        char *dash = strchr(line, '-');
        if (!dash) {
            continue;
        }

        *dash = '\0';
        const char *id_str   = line + 1;
        const char *name_str = dash + 1;

        int id = atoi(id_str);
        if (id <= 0) {
            continue;
        }

        instrumentos_id[total_instrumentos] = (uint8_t)id;
        strncpy(instrumentos[total_instrumentos], name_str, MAX_NOMBRE_INSTR);
        instrumentos[total_instrumentos][MAX_NOMBRE_INSTR] = '\0';

        total_instrumentos++;
    }

    f_close(&file);

    printf("Instrumentos cargados desde index.txt: %u\n", total_instrumentos);
    instrumentos_debug();
}

/**
 * @brief Imprime la tabla de instrumentos por consola para depuración.
 */
void instrumentos_debug(void) {
    printf("Tabla de instrumentos:\n");
    for (uint8_t i = 0; i < total_instrumentos; i++) {
        printf("  [%u] id=%u, nombre=%s\n",
               i, instrumentos_id[i], instrumentos[i]);
    }
}

/* -------------------------------------------------------------------------
 * Sistema de slots (parte simple)
 *
 * Variables globales para el sistema que administra dos "slots" (H/V).
 * Se asume que SLOT_H y SLOT_V están definidos en sistema.h.
 * ------------------------------------------------------------------------- */

/** @brief Slot actualmente activo (SLOT_H o SLOT_V). */
uint8_t slot_activo = SLOT_H;

/** @brief Instrumentos asignados a cada slot (índices en la tabla de instrumentos). */
uint8_t instrumento_slot[2] = {0, 1};

/**
 * @brief Inicializa el sistema de slots en función de la cantidad de instrumentos.
 *
 * - Si no hay instrumentos, ambos slots apuntan a 0.
 * - Si hay uno, ambos slots apuntan al mismo instrumento.
 * - Si hay >1, asigna 0 y 1 como iniciales.
 *
 * También refresca la pantalla llamando a lcd_mostrar_estado().
 */
void sistema_init(void) {
    if (total_instrumentos == 0) {
        instrumento_slot[SLOT_H] = 0;
        instrumento_slot[SLOT_V] = 0;
    } else if (total_instrumentos == 1) {
        instrumento_slot[SLOT_H] = 0;
        instrumento_slot[SLOT_V] = 0;
    } else {
        instrumento_slot[SLOT_H] = 0;
        instrumento_slot[SLOT_V] = 1;
    }

    lcd_mostrar_estado();
}

/**
 * @brief Procesa eventos de botones relacionados con el sistema de selección.
 *
 * Atiende los eventos BTN_SLOT_EVT, BTN_NEXT_EVT y BTN_PREV_EVT provistos por botones_get_evento().
 * Según el evento actualiza el slot activo o cambia el instrumento asignado al slot.
 *
 * @return true si hubo actividad (cambio de estado), false si no ocurrió nada.
 */
bool sistema_update(void) {
    boton_evento_t ev = botones_get_evento();
    bool actividad = false;

    switch (ev) {
        case BTN_SLOT_EVT:
            slot_activo ^= 1;
            actividad = true;
            lcd_mostrar_estado();
            break;

        case BTN_NEXT_EVT:
            if (total_instrumentos > 0) {
                instrumento_slot[slot_activo] =
                    (instrumento_slot[slot_activo] + 1) % total_instrumentos;
                actividad = true;
                lcd_mostrar_estado();
            }
            break;

        case BTN_PREV_EVT:
            if (total_instrumentos > 0) {
                if (instrumento_slot[slot_activo] == 0) {
                    instrumento_slot[slot_activo] = total_instrumentos - 1;
                } else {
                    instrumento_slot[slot_activo]--;
                }
                actividad = true;
                lcd_mostrar_estado();
            }
            break;

        default:
            break;
    }

    return actividad;
}
