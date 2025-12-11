#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <stdbool.h>
#include "ff.h"

// Inicializar módulo SD con DMA
bool sd_manager_init();

// Desinicializar módulo SD
void sd_manager_deinit();

// Verificar si SD está inicializada
bool sd_manager_is_ready();

// Listar archivos .wav en la SD
void sd_manager_list_wav_files();

#endif // SD_MANAGER_H