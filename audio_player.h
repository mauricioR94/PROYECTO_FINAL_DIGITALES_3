/**
 * @file audio_player.h
 * @brief Módulo de reproducción de audio WAV usando doble buffer e I2S.
 * @authors 
 *  - Mauricio Reyes Rosero
 *  - Reinaldo Marín Nieto
 *  - Daniel Pérez Gallego
 *  - Jorge Arroyo Niño
 */

#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <stdint.h>
#include <stdbool.h>

/** Tamaño de cada buffer de audio (8 KB). */
#define AUDIO_BUFFER_SIZE 8192

/**
 * @brief Estados posibles del reproductor de audio.
 */
typedef enum {
    PLAYER_IDLE,      /**< Sin archivo cargado. */
    PLAYER_LOADING,   /**< Cargando archivo desde SD. */
    PLAYER_PLAYING,   /**< Reproducción en curso. */
    PLAYER_PAUSED,    /**< Reproducción pausada. */
    PLAYER_ERROR      /**< Error crítico. */
} player_state_t;

/**
 * @brief Estructura con información del reproductor.
 */
typedef struct {
    player_state_t state;      /**< Estado actual del reproductor. */
    uint32_t bytes_played;     /**< Bytes reproducidos hasta el momento. */
    uint32_t total_bytes;      /**< Tamaño total del audio. */
    uint32_t sample_rate;      /**< Frecuencia de muestreo del archivo WAV. */
    uint16_t num_channels;     /**< Número de canales (1 o 2). */
    uint16_t bits_per_sample;  /**< Resolución en bits (solo 16). */
    float progress_percent;    /**< Porcentaje de progreso. */
} player_info_t;

/**
 * @brief Inicializa el reproductor y su estado interno.
 * @return true si la inicialización fue exitosa.
 */
bool audio_player_init();

/**
 * @brief Inicia la reproducción de un archivo WAV desde la SD.
 * @param filename Nombre del archivo en la tarjeta SD.
 * @return true si pudo comenzar la reproducción.
 */
bool audio_player_play(const char *filename);

/**
 * @brief Detiene la reproducción actual.
 */
void audio_player_stop();

/**
 * @brief Pausa la reproducción.
 */
void audio_player_pause();

/**
 * @brief Reanuda la reproducción si estaba pausada.
 */
void audio_player_resume();

/**
 * @brief Obtiene la información actual del reproductor.
 * @return player_info_t con datos del estado.
 */
player_info_t audio_player_get_info();

/**
 * @brief Indica si el reproductor está en modo PLAYING.
 * @return true si está reproduciendo.
 */
bool audio_player_is_playing();

/**
 * @brief Proceso continuo que se debe llamar frecuentemente
 * para enviar frames al I2S y realizar gestión de buffers.
 */
void audio_player_process();

#endif // AUDIO_PLAYER_H
