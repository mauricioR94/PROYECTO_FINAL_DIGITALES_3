#ifndef BOTONES_H
#define BOTONES_H

#include <stdint.h>

#define BTN_SLOT 10
#define BTN_NEXT 11
#define BTN_PREV 12

typedef enum {
    BTN_SLOT_EVT,
    BTN_NEXT_EVT,
    BTN_PREV_EVT,
    BTN_NONE
} boton_evento_t;

void botones_init();
void botones_update();
boton_evento_t botones_get_evento();

#endif
