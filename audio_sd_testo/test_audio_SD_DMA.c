#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "sd_manager.h"
#include "i2s_output.h"
#include "audio_player.h"
#include "button_controller.h"
#include "mpu6050.h"

int main() {
    stdio_init_all();
    sleep_ms(3000);
    
    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║      HANDINO MOTION TOOL - PROTOTIPO      ║\n");
    printf("║  SD + I2S + BOTONES + IMU (MPU6050)       ║\n");
    printf("╚════════════════════════════════════════════╝\n\n");
    
    // PASO 1: Inicializar módulo SD
    printf("PASO 1: Inicializar módulo SD\n");
    if (!sd_manager_init()) {
        printf("\n❌ FALLO: No se pudo inicializar la SD\n");
        while (1) {
            sleep_ms(1000);
        }
    }
    printf("\n");
    sleep_ms(500);
    
    // PASO 2: Verificar archivos de audio
    printf("PASO 2: Verificar archivos de audio necesarios\n");
    printf("Estructura esperada en la SD (ejemplos):\n");
    printf("  i1a-do.wav, i1a-re.wav, ... i1a-si.wav\n");
    printf("  i1b-do.wav, i1b-re.wav, ... i1b-si.wav\n");
    printf("  i2a-do.wav, i2a-re.wav, ... i2a-si.wav\n");
    printf("  i2b-do.wav, i2b-re.wav, ... i2b-si.wav\n\n");
    
    sd_manager_list_wav_files();
    sleep_ms(500);
    
    // PASO 3: Inicializar salida I2S
    printf("PASO 3: Inicializar salida I2S (DAC UDA1334A)\n");
    if (!i2s_output_init(44100)) {  // 44.1 kHz inicial de referencia
        printf("❌ Error al inicializar I2S\n");
        return -1;
    }
    printf("\n");
    sleep_ms(500);
    
    // PASO 4: Inicializar reproductor
    printf("PASO 4: Inicializar reproductor de audio\n");
    if (!audio_player_init()) {
        printf("❌ Error al inicializar reproductor\n");
        return -1;
    }
    printf("\n");
    sleep_ms(500);
    
    // PASO 5: Inicializar botones
    printf("PASO 5: Inicializar controlador de botones\n");
    button_controller_init();
    sleep_ms(500);

    // PASO 6: Inicializar IMU (MPU6050)
    printf("PASO 6: Inicializar MPU6050 (IMU)\n");
    // Usamos i2c0, SDA=GPIO4, SCL=GPIO5 (como en los tests)
    mpu6050_init(i2c0, 4, 5);
    printf("✅ MPU6050 inicializado en I2C0 (SDA=4, SCL=5)\n\n");
    sleep_ms(500);
    
    // Sistema listo
    printf("╔════════════════════════════════════════════╗\n");
    printf("║        🎹 SISTEMA LISTO 🎹                 ║\n");
    printf("╚════════════════════════════════════════════╝\n\n");
    
    printf("Instrucciones:\n");
    printf("  - Orientación horizontal  -> Instrumento 1\n");
    printf("  - Orientación vertical    -> Instrumento 2\n");
    printf("  - Girar/sacudir en yaw (eje Z) sostenido  -> Sonido B\n");
    printf("  - Estático en yaw                              -> Sonido A\n");
    printf("  - Botones Do..Si                               -> Nota a reproducir\n\n");
    
    printf("Conexiones de botones:\n");
    printf("  GPIO 18 -> Botón Do  (otro extremo a GND)\n");
    printf("  GPIO 20 -> Botón Re  (otro extremo a GND)\n");
    printf("  GPIO 19 -> Botón Mi  (otro extremo a GND)\n");
    printf("  GPIO  9 -> Botón Fa  (otro extremo a GND)\n");
    printf("  GPIO  8 -> Botón Sol (otro extremo a GND)\n");
    printf("  GPIO  7 -> Botón La  (otro extremo a GND)\n");
    printf("  GPIO  6 -> Botón Si  (otro extremo a GND)\n\n");
    
    printf("Conexiones de IMU (MPU6050):\n");
    printf("  GPIO 4  -> SDA\n");
    printf("  GPIO 5  -> SCL\n");
    printf("  3V3     -> VCC\n");
    printf("  GND     -> GND\n");
    printf("  AD0     -> GND (dir 0x68)\n\n");
    
    printf("Esperando pulsaciones y movimiento.\n");
    printf("════════════════════════════════════════════\n\n");
    
    uint32_t samples_processed = 0;
    uint32_t last_status_time = 0;
    
    // Estado del instrumento y sonido según IMU
    bool instrumento2 = false;   // false -> instrumento 1, true -> instrumento 2
    bool sonido_b     = false;   // false -> A, true -> B
    
    uint32_t last_imu_time   = 0;   // para limitar la tasa de lectura de IMU
    uint32_t yaw_strong_time = 0;   // tiempo acumulado con yaw fuerte (ms aprox)
    
    // Tabla de notas para nombres de archivo
    const char *note_tokens[7] = {
        "do", "re", "mi", "fa", "sol", "la", "si"
    };
    
    while (true) {
        // 1. Procesar reproducción actual (PRIORIDAD MÁXIMA)
        for (int i = 0; i < 8; i++) {  // Procesar múltiples muestras por iteración
            audio_player_process();
        }
        samples_processed += 8;
        
        // 2. Actualizar IMU periódicamente (por ejemplo cada 50 ms)
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_imu_time > 50) {
            uint32_t dt = now - last_imu_time;   // tiempo desde la última lectura
            last_imu_time = now;
            
            mpu6050_raw_t data;
            mpu6050_read_raw(&data);
            
            float ax = mpu6050_calc_g(data.ax);
            float ay = mpu6050_calc_g(data.ay);
            float az = mpu6050_calc_g(data.az);
            
            float pitch = calc_pitch(ax, ay, az);
            float roll  = calc_roll(ay, az);
            
            bool vertical   = is_vertical(pitch, roll);
            bool horizontal = is_horizontal(pitch, roll);
            
            // === Cambio de INSTRUMENTO (igual que antes, usando pitch/roll) ===
            if (!instrumento2 && vertical) {
                instrumento2 = true;
                printf("🔄 Cambiando a INSTRUMENTO 2 (vertical)\n");
            } else if (instrumento2 && horizontal) {
                instrumento2 = false;
                printf("🔄 Volviendo a INSTRUMENTO 1 (horizontal)\n");
            }
            
            // === Cambio de SONIDO A/B usando YAW (giroscopio gz) ===
            //
            // Usamos el giro en Z en °/s, con histéresis y tiempo sostenido.
            // Conversión: por defecto el MPU6050 usa ±250°/s -> 131 LSB por °/s.
            const float    YAW_ON_THRESHOLD_DPS  = 35.0f;   // activar B (rotación clara)
            const float    YAW_OFF_THRESHOLD_DPS = 15.0f;   // volver a A (zona muerta)
            const uint32_t YAW_SUSTAIN_MS        = 300;     // tiempo sostenido (ms)
            
            float gz_dps  = (float)data.gz / 131.0f;   // °/s aprox en yaw (eje Z)
            float yaw_abs = fabsf(gz_dps);
            
            bool old_sonido_b = sonido_b;
            
            if (yaw_abs > YAW_ON_THRESHOLD_DPS) {
                // Movimiento fuerte → acumular tiempo
                yaw_strong_time += dt;
                if (yaw_strong_time >= YAW_SUSTAIN_MS) {
                    sonido_b = true;
                }
            } else if (yaw_abs < YAW_OFF_THRESHOLD_DPS) {
                // Muy poco movimiento → volver a A y resetear
                yaw_strong_time = 0;
                sonido_b = false;
            } else {
                // Entre ON y OFF: zona de histéresis
                if (!sonido_b) {
                    // seguimos esperando un yaw realmente fuerte
                    yaw_strong_time = 0;
                }
            }
            
            // Mensajes solo cuando cambie A/B
            if (sonido_b != old_sonido_b) {
                if (!instrumento2) {
                    printf("Instrumento 1 → SONIDO %c (%s yaw sostenido, |yaw|=%.1f °/s)\n",
                           sonido_b ? 'B' : 'A',
                           sonido_b ? "con" : "sin",
                           yaw_abs);
                } else {
                    printf("Instrumento 2 → SONIDO %c (%s yaw sostenido, |yaw|=%.1f °/s)\n",
                           sonido_b ? 'B' : 'A',
                           sonido_b ? "con" : "sin",
                           yaw_abs);
                }
            }
        }
        
        // 3. Mostrar estado cada 5 segundos (solo cuando está reproduciendo)
        if (audio_player_is_playing() && (now - last_status_time > 5000)) {
            player_info_t info = audio_player_get_info();
            printf("   📊 Progreso: %.1f%% (%lu/%lu bytes, %lu muestras procesadas)\n", 
                   info.progress_percent, info.bytes_played, info.total_bytes, samples_processed);
            last_status_time = now;
        }
        
        // 4. Verificar botones (permite tocar mientras el instrumento/sonido
        //    se define por la IMU)
        int pressed_button = button_controller_process();
        
        // Permitimos repetir la misma nota las veces que sea
        if (pressed_button >= 0) {
            const char *note_name = button_controller_get_note_name(pressed_button);
            
            // Mapear a nombre de archivo según IMU
            int  instrumento = instrumento2 ? 2 : 1;
            char sound_char  = sonido_b ? 'b' : 'a';
            const char *note_token = "do";
            
            if (pressed_button >= 0 && pressed_button < 7) {
                note_token = note_tokens[pressed_button];
            }
            
            char wav_file[32];
            snprintf(wav_file, sizeof(wav_file),
                     "0:/i%d%c-%s.wav", instrumento, sound_char, note_token);
            
            printf("\n🎵 Nota: %s -> Archivo: %s\n", note_name, wav_file);
            
            // Detener reproducción actual (si existe)
            if (audio_player_is_playing()) {
                audio_player_stop();
            }
            
            // Reproducir nuevo archivo
            if (!audio_player_play(wav_file)) {
                printf("⚠️  Advertencia: No se encontró '%s'\n", wav_file);
                printf("   Verifica que el archivo exista en la SD\n");
            } else {
                samples_processed = 0;  // Resetear contador
                last_status_time  = now;
            }
        }
        
        // 5. Pequeña pausa para estabilidad
        sleep_us(50);  // 50 microsegundos = 20,000 iteraciones/segundo
    }
    
    return 0;
}
