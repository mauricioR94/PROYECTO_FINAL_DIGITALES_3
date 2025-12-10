#ifndef SISTEMA_H
#define SISTEMA_H

#include <stdint.h>

#define SLOT_H 0
#define SLOT_V 1
#define TOTAL_INSTRUMENTOS 10

extern uint8_t slot_activo;
extern uint8_t instrumento_slot[2];

void sistema_init();
void sistema_update();
void iniciar_calibracion(void);

#endif
