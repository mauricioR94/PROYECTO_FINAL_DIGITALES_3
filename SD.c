#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "ff.h"
#include "hw_config.h"

// Función para escribir un archivo
bool escribir_archivo(const char *filename, const char *contenido) {
    FIL file;
    FRESULT fr;
    UINT bytes_written;
    
    // Abrir archivo para escritura (crea si no existe)
    fr = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        printf("Error al abrir archivo para escritura: %d\n", fr);
        return false;
    }
    
    // Escribir contenido
    fr = f_write(&file, contenido, strlen(contenido), &bytes_written);
    if (fr != FR_OK) {
        printf("Error al escribir: %d\n", fr);
        f_close(&file);
        return false;
    }
    
    printf("Escritos %u bytes en '%s'\n", bytes_written, filename);
    
    // Cerrar archivo
    f_close(&file);
    return true;
}

// Función para leer un archivo
bool leer_archivo(const char *filename) {
    FIL file;
    FRESULT fr;
    char buffer[256];
    UINT bytes_read;
    
    // Abrir archivo para lectura
    fr = f_open(&file, filename, FA_READ);
    if (fr != FR_OK) {
        printf("Error al abrir archivo para lectura: %d\n", fr);
        return false;
    }
    
    printf("Contenido de '%s':\n", filename);
    printf("------------------------\n");
    
    // Leer y mostrar contenido
    while (f_read(&file, buffer, sizeof(buffer) - 1, &bytes_read) == FR_OK && bytes_read > 0) {
        buffer[bytes_read] = '\0';  // Terminar string
        printf("%s", buffer);
    }
    
    printf("\n------------------------\n");
    
    // Cerrar archivo
    f_close(&file);
    return true;
}

// Función para listar archivos en el directorio raíz
void listar_archivos() {
    DIR dir;
    FILINFO fno;
    FRESULT fr;
    
    printf("\nArchivos en la SD:\n");
    printf("------------------------\n");
    
    fr = f_opendir(&dir, "/");
    if (fr != FR_OK) {
        printf("Error al abrir directorio: %d\n", fr);
        return;
    }
    
    while (true) {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) break;  // Error o fin
        
        if (fno.fattrib & AM_DIR) {
            printf("[DIR]  %s\n", fno.fname);
        } else {
            printf("[FILE] %s (%lu bytes)\n", fno.fname, fno.fsize);
        }
    }
    
    f_closedir(&dir);
    printf("------------------------\n");
}

int main() {
    // Inicializar entrada/salida estándar
    stdio_init_all();
    sleep_ms(3000);  // Esperar a que se conecte la consola USB
    
    printf("\n=================================\n");
    printf("  Prueba de tarjeta SD - RP2040\n");
    printf("=================================\n\n");
    
    // Inicializar tarjeta SD
    if (!sd_init()) {
        printf("FALLO: No se pudo inicializar la SD\n");
        while (1) {
            sleep_ms(1000);
        }
    }
    
    printf("\n--- Prueba de escritura ---\n");
    if (escribir_archivo("0:/prueba.txt", "Hola desde RP2040!\nEsto es una prueba de escritura.\n")) {
        printf("Archivo escrito exitosamente\n");
    }
    
    sleep_ms(500);
    
    printf("\n--- Prueba de lectura ---\n");
    leer_archivo("0:/prueba.txt");
    
    sleep_ms(500);
    
    printf("\n--- Listado de archivos ---\n");
    listar_archivos();
    
    printf("\n=================================\n");
    printf("  Prueba completada\n");
    printf("=================================\n");
    
    // Desinicializar SD
    sd_deinit();
    
    // Loop infinito
    while (1) {
        tight_loop_contents();
    }
    
    return 0;
}