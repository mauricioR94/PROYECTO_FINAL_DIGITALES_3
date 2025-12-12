/**
 * @file sd_manager.c
 * @brief Implementación del módulo SD con acceso FATFS y SPI DMA.
 *
 * Contiene lógica de inicialización, desmontaje, verificación de estado
 * y listado de archivos .wav.
 */

#include "sd_manager.h"
#include "hw_config.h"
#include <stdio.h>
#include <string.h>

/** @brief Indica si la tarjeta SD ya fue montada correctamente. */
static bool sd_ready = false;

/**
 * @brief Inicializa y monta la tarjeta SD.
 *
 * Obtiene la configuración desde hw_config, inicializa SPI,
 * monta la partición FATFS y verifica errores.
 *
 * @return true si el montaje fue exitoso.
 */
bool sd_manager_init() {
    printf("Inicializando módulo SD con DMA...\n");
    
    sd_card_t *pSD = sd_get_by_num(0);
    if (!pSD) {
        printf("No se encontró configuración de SD\n");
        return false;
    }
    
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (fr != FR_OK) {
        printf("Error al montar SD (código: %d)\n", fr);
        printf("\nVerifica las conexiones:\n");
        printf("  GPIO 22 CS\n");
        printf("  GPIO 27  MOSI\n");
        printf("  GPIO 26  CLK\n");
        printf("  GPIO 28 MISO\n");
        printf("  3.3V     VCC\n");
        printf("  GND    GND\n");
        return false;
    }
    
    sd_ready = true;
    printf(" SD montada exitosamente en '%s'\n", pSD->pcName);
    printf("   Velocidad SPI: %.1f MHz\n", SD_SPI_BAUDRATE / 1000000.0);
    return true;
}

/**
 * @brief Desmonta la SD si está activa y limpia el estado interno.
 */
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

/**
 * @brief Indica si la tarjeta SD está inicializada y lista.
 */
bool sd_manager_is_ready() {
    return sd_ready;
}

/**
 * @brief Lista todos los archivos .wav presentes en el directorio raíz.
 *
 * Imprime nombre y tamaño de cada archivo. También muestra un mensaje
 * si no se encontraron archivos útiles.
 */
void sd_manager_list_wav_files() {
    if (!sd_ready) {
        printf("SD no está lista\n");
        return;
    }
    
    DIR dir;
    FILINFO fno;
    FRESULT fr;
    
    printf("\n Archivos WAV en la SD \n");
    
    fr = f_opendir(&dir, "0:/");
    if (fr != FR_OK) {
        printf("Error al abrir directorio: %d\n", fr);
        return;
    }
    
    int count = 0;
    while (true) {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) break;
        
        // Buscar extensión .wav o .WAV
        size_t len = strlen(fno.fname);
        if (len > 4 &&
            (strcmp(&fno.fname[len-4], ".wav") == 0 ||
             strcmp(&fno.fname[len-4], ".WAV") == 0)) {
            printf("  [%d] %s (%lu bytes)\n",
                   count++, fno.fname, fno.fsize);
        }
    }
    
    f_closedir(&dir);
    
    if (count == 0) {
        printf("  No se encontraron archivos .wav\n");
    }
}
