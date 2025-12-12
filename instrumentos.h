/**
 * @file instrumentos.h
 * @brief Gestión de la tabla de instrumentos disponibles (index.txt).
 *
 * Permite cargar desde "0:/index.txt" una lista de instrumentos en formato:
 *   iN-NombreDelInstrumento
 * donde N es un id (entero) y NombreDelInstrumento es la etiqueta.
 *
 * Provee utilidades para mantener una tabla en RAM y volcarla por consola.
 *
 * @authors
 *  - Mauricio Reyes Rosero
 *  - Reinaldo Marín Nieto
 *  - Daniel Pérez Gallego
 *  - Jorge Arroyo Niño
 */

#ifndef INSTRUMENTOS_H
#define INSTRUMENTOS_H

#include <stdint.h>

#define MAX_INSTRUMENTOS   10            /**< Máximo número de instrumentos. */
#define MAX_NOMBRE_INSTR   16            /**< Longitud visible en LCD (+1 para '\0'). */

/** @brief Tabla de nombres de instrumento (externa). */
extern char    instrumentos[MAX_INSTRUMENTOS][MAX_NOMBRE_INSTR + 1];

/** @brief IDs asociados a cada entrada de la tabla (1..n). */
extern uint8_t instrumentos_id[MAX_INSTRUMENTOS];

/** @brief Cantidad actual de instrumentos cargados. */
extern uint8_t total_instrumentos;

/**
 * @brief Lee 0:/index.txt y llena la tabla de instrumentos e IDs.
 *
 * El archivo debe tener líneas del formato "i<id>-<nombre>".
 * Las entradas inválidas se ignoran. Se detiene al alcanzar MAX_INSTRUMENTOS.
 */
void cargar_instrumentos_desde_index(void);

/**
 * @brief Imprime por consola la tabla de instrumentos (debug).
 */
void instrumentos_debug(void);

#endif // INSTRUMENTOS_H
