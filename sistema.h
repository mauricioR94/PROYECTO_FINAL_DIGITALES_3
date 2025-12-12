#ifndef SISTEMA_H
#define SISTEMA_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Identificador para el slot horizontal.
 */
#define SLOT_H 0

/**
 * @brief Identificador para el slot vertical.
 */
#define SLOT_V 1

/**
 * @brief Slot actualmente activo (horizontal o vertical).
 */
extern uint8_t slot_activo;

/**
 * @brief Instrumento asignado a cada slot.
 * 
 * índice 0: SLOT_H  
 * índice 1: SLOT_V
 */
extern uint8_t instrumento_slot[2];

/**
 * @brief Inicializa el sistema, variables globales y estado base.
 */
void sistema_init(void);

/**
 * @brief Actualiza el sistema según entradas o cambios de estado.
 * 
 * @return true si hubo un cambio significativo o evento.
 * @return false si todo sigue igual.
 */
bool sistema_update(void);

#endif
