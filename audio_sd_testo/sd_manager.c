#include "sd_manager.h"
#include "hw_config.h"
#include <stdio.h>
#include <string.h>

static bool sd_ready = false;

bool sd_manager_init() {
    printf("Inicializando módulo SD con DMA...\n");
    
    sd_card_t *pSD = sd_get_by_num(0);
    if (!pSD) {
        printf("✗ Error: No se encontró configuración de SD\n");
        return false;
    }
    
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (fr != FR_OK) {
        printf("✗ Error al montar SD (código: %d)\n", fr);
        printf("\nVerifica las conexiones:\n");
        printf("  GPIO 22 -> CS\n");
        printf("  GPIO 27 -> MOSI\n");
        printf("  GPIO 26 -> CLK\n");
        printf("  GPIO 28 -> MISO\n");
        printf("  3.3V    -> VCC\n");
        printf("  GND     -> GND\n");
        return false;
    }
    
    sd_ready = true;
    printf("✅ SD montada exitosamente en '%s'\n", pSD->pcName);
    printf("   Velocidad SPI: %.1f MHz\n", SD_SPI_BAUDRATE / 1000000.0);
    return true;
}

void sd_manager_deinit() {
    if (sd_ready) {
        sd_card_t *pSD = sd_get_by_num(0);
        if (pSD) {
            f_unmount(pSD->pcName);
            sd_ready = false;
            printf("SD desmontada\n");
        }
    }
}

bool sd_manager_is_ready() {
    return sd_ready;
}

void sd_manager_list_wav_files() {
    if (!sd_ready) {
        printf("SD no está lista\n");
        return;
    }
    
    DIR dir;
    FILINFO fno;
    FRESULT fr;
    
    printf("\n=== Archivos WAV en la SD ===\n");
    
    fr = f_opendir(&dir, "0:/");
    if (fr != FR_OK) {
        printf("Error al abrir directorio: %d\n", fr);
        return;
    }
    
    int count = 0;
    while (true) {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) break;
        
        // Buscar extensión .wav
        size_t len = strlen(fno.fname);
        if (len > 4 && 
            (strcmp(&fno.fname[len-4], ".wav") == 0 || 
             strcmp(&fno.fname[len-4], ".WAV") == 0)) {
            printf("  [%d] %s (%lu bytes)\n", count++, fno.fname, fno.fsize);
        }
    }
    
    f_closedir(&dir);
    
    if (count == 0) {
        printf("  No se encontraron archivos .wav\n");
    }
    printf("=============================\n\n");
}