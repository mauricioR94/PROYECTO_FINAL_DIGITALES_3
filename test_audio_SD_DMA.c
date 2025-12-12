/**
 * @file test_audio_SD_DMA.c
 * @brief Prueba completa del sistema de audio: SD, I2S, reproductor, LCD,
 *        selector de instrumentos, IMU y modo de bajo consumo.
 */

#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/clocks.h"

#include "sd_manager.h"
#include "i2s_output.h"
#include "audio_player.h"
#include "button_controller.h"
#include "mpu6050.h"
#include "lcd.h"
#include "botones.h"
#include "sistema.h"
#include "instrumentos.h"

/**
 * @brief Tiempo máximo de inactividad antes de entrar en modo de bajo consumo (10 min).
 */
#define INACTIVITY_MS (10u * 60u * 1000u)

/**
 * @brief Indica si el sistema está en modo de bajo consumo.
 */
static bool low_power_mode = false;

/**
 * @brief Marca de tiempo de la última actividad detectada.
 */
static uint32_t last_activity_time = 0;

/**
 * @brief Ingresa al modo de bajo consumo reduciendo la frecuencia del sistema
 *        y deteniendo el reproductor de audio.
 */
static void enter_low_power_mode(void) {
    if (low_power_mode) {
        return;
    }

    audio_player_stop();
    set_sys_clock_khz(12000, true);
    low_power_mode = true;
}

/**
 * @brief Sale del modo de bajo consumo restaurando la frecuencia del sistema
 *        y actualizando el tiempo de actividad.
 *
 * @param now Marca de tiempo actual en milisegundos.
 */
static void exit_low_power_mode(uint32_t now) {
    if (!low_power_mode) {
        last_activity_time = now;
        return;
    }

    set_sys_clock_khz(125000, true);
    low_power_mode = false;
    last_activity_time = now;
}

/**
 * @brief Punto de entrada principal del sistema de audio. Inicializa todos los
 *        módulos (SD, I2S, reproductor, IMU, LCD, botones) y coordina la lógica
 *        de reproducción, selección de instrumentos y modos de energía.
 *
 * @return int Código de retorno estándar.
 */
int main(void) {
    stdio_init_all();
    sleep_ms(3000);

    printf("\n");
    printf("Handino Motion Tool\n");
    printf("Inicializando sistema...\n\n");

    printf("Paso 1: Inicializar modulo SD\n");
    if (!sd_manager_init()) {
        printf("Error: no se pudo inicializar la SD\n");
        while (1) {
            sleep_ms(1000);
        }
    }
    printf("Modulo SD listo\n\n");
    sleep_ms(300);

    printf("Paso 2: Verificar archivos de audio en SD\n");
    sd_manager_list_wav_files();
    printf("Fin de listado de archivos\n\n");
    sleep_ms(300);

    printf("Paso 2b: Cargar instrumentos desde index.txt\n");
    cargar_instrumentos_desde_index();
    printf("Instrumentos cargados\n\n");
    sleep_ms(300);

    printf("Paso 3: Inicializar salida I2S\n");
    if (!i2s_output_init(44100)) {
        printf("Error al inicializar I2S\n");
        return -1;
    }
    printf("Salida I2S inicializada\n\n");
    sleep_ms(300);

    printf("Paso 4: Inicializar reproductor de audio\n");
    if (!audio_player_init()) {
        printf("Error al inicializar reproductor de audio\n");
        return -1;
    }
    printf("Reproductor de audio inicializado\n\n");
    sleep_ms(300);

    printf("Paso 5: Inicializar botones de notas\n");
    button_controller_init();
    printf("Botones de notas listos\n\n");
    sleep_ms(300);

    printf("Paso 6: Inicializar MPU6050 (IMU)\n");
    mpu6050_init(i2c0, 4, 5);
    printf("MPU6050 inicializado en I2C0 (SDA=4, SCL=5)\n\n");
    sleep_ms(300);

    printf("Paso 7: Inicializar LCD y selector de instrumentos\n");
    lcd_init();
    botones_init();
    sistema_init();
    printf("LCD y selector de instrumentos inicializados\n\n");
    sleep_ms(300);

    uint32_t samples_processed = 0;
    uint32_t last_status_time  = 0;

    bool instrumento2 = false;
    bool sonido_b     = false;

    uint32_t last_imu_time   = 0;
    uint32_t yaw_strong_time = 0;

    const char *note_tokens[7] = {
        "do", "re", "mi", "fa", "sol", "la", "si"
    };

    last_activity_time = to_ms_since_boot(get_absolute_time());
    low_power_mode = false;

    printf("Sistema listo. Use los botones de notas y el selector de instrumentos.\n\n");

    while (1) {
        uint32_t now = to_ms_since_boot(get_absolute_time());

        /**
         * @brief Procesamiento continuo del reproductor de audio mediante polling.
         */
        for (int i = 0; i < 8; i++) {
            audio_player_process();
        }
        samples_processed += 8;

        /**
         * @brief Lectura periódica de la IMU para detectar orientación y giros.
         */
        if (now - last_imu_time > 50) {
            uint32_t dt = (last_imu_time == 0) ? 0 : (now - last_imu_time);
            last_imu_time = now;

            mpu6050_raw_t data;
            mpu6050_read_raw(&data);

            float ax = mpu6050_calc_g(data.ax);
            float ay = mpu6050_calc_g(data.ay);
            float az = mpu6050_calc_g(data.az);

            float pitch = calc_pitch(ax, ay, az);
            float roll  = calc_roll(ay, az);

            /**
             * @brief Interpretación de orientación para decidir si usar SLOT_H o SLOT_V.
             */
            bool vertical   = is_vertical(pitch, roll);
            bool horizontal = is_horizontal(pitch, roll);

            if (!instrumento2 && vertical) {
                instrumento2 = true;
            } else if (instrumento2 && horizontal) {
                instrumento2 = false;
            }

            /**
             * @brief Lógica de detección de giro fuerte en yaw para activar sonido 'b'.
             */
            const float    YAW_ON_THRESHOLD_DPS  = 35.0f;
            const float    YAW_OFF_THRESHOLD_DPS = 15.0f;
            const uint32_t YAW_SUSTAIN_MS        = 300;

            float gz_dps  = (float)data.gz / 131.0f;
            float yaw_abs = fabsf(gz_dps);

            if (dt > 0) {
                if (yaw_abs > YAW_ON_THRESHOLD_DPS) {
                    yaw_strong_time += dt;
                    if (yaw_strong_time >= YAW_SUSTAIN_MS) {
                        sonido_b = true;
                    }
                } else if (yaw_abs < YAW_OFF_THRESHOLD_DPS) {
                    yaw_strong_time = 0;
                    sonido_b = false;
                } else {
                    if (!sonido_b) {
                        yaw_strong_time = 0;
                    }
                }
            }
        }

        /**
         * @brief Gestión del selector de instrumentos.
         */
        botones_update();
        if (sistema_update()) {
            exit_low_power_mode(now);
        }

        /**
         * @brief Estado cada 5 s si un archivo está en reproducción.
         */
        if (audio_player_is_playing() && (now - last_status_time > 5000)) {
            player_info_t info = audio_player_get_info();
            printf("Progreso: %.1f%% (%lu/%lu bytes, %lu muestras procesadas)\n",
                   info.progress_percent,
                   (unsigned long)info.bytes_played,
                   (unsigned long)info.total_bytes,
                   (unsigned long)samples_processed);
            last_status_time = now;
        }

        /**
         * @brief Procesamiento de botones de notas.
         */
        int pressed_button = button_controller_process();

        if (pressed_button >= 0) {
            exit_low_power_mode(now);

            const char *note_name = button_controller_get_note_name(pressed_button);

            uint8_t slot = instrumento2 ? SLOT_V : SLOT_H;
            uint8_t idx  = instrumento_slot[slot];

            if (idx >= total_instrumentos && total_instrumentos > 0) {
                idx = 0;
            }

            uint8_t inst_id = (total_instrumentos > 0) ? instrumentos_id[idx] : 1;
            const char *note_token = "do";

            if (pressed_button >= 0 && pressed_button < 7) {
                note_token = note_tokens[pressed_button];
            }

            char sound_char = sonido_b ? 'b' : 'a';

            char wav_file[40];
            snprintf(wav_file, sizeof(wav_file),
                     "0:/i%u%c-%s.wav", inst_id, sound_char, note_token);

            printf("Nota: %s | Instrumento id=%u | Sonido %c | Archivo: %s\n",
                   note_name,
                   inst_id,
                   sound_char,
                   wav_file);

            if (audio_player_is_playing()) {
                audio_player_stop();
            }

            if (audio_player_play(wav_file)) {
                samples_processed = 0;
                last_status_time  = now;
            } else {
                printf("Advertencia: no se encontro el archivo %s\n", wav_file);
            }
        }

        /**
         * @brief Entrada al modo de bajo consumo por inactividad.
         */
        if (!low_power_mode &&
            !audio_player_is_playing() &&
            (now - last_activity_time >= INACTIVITY_MS)) {
            enter_low_power_mode();
            printf("Entrando en modo de bajo consumo tras inactividad prolongada.\n");
        }

        /**
         * @brief Pequeño descanso para suavizar el polling.
         */
        sleep_us(50);
    }

    return 0;
}
