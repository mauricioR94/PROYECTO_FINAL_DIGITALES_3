#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#include "ff.h"
#include "sd_card.h"

// Funciones requeridas por la librería FatFS
size_t sd_get_num();
sd_card_t *sd_get_by_num(size_t num);

// Funciones auxiliares para tu aplicación
bool sd_init();
void sd_deinit();

#endif // HW_CONFIG_H