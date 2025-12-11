#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <stdint.h>
#include <stdbool.h>

// Configuración del reproductor
#define AUDIO_BUFFER_SIZE 8192  // 8KB por buffer (double-buffering)

// Estados del reproductor
typedef enum {
    PLAYER_IDLE,
    PLAYER_LOADING,
    PLAYER_PLAYING,
    PLAYER_PAUSED,
    PLAYER_ERROR
} player_state_t;

// Información del reproductor
typedef struct {
    player_state_t state;
    uint32_t bytes_played;
    uint32_t total_bytes;
    uint32_t sample_rate;
    uint16_t num_channels;
    uint16_t bits_per_sample;
    float progress_percent;
} player_info_t;

// Inicializar reproductor (requiere I2S ya inicializado)
bool audio_player_init();

// Reproducir archivo WAV desde SD
bool audio_player_play(const char *filename);

// Control de reproducción
void audio_player_stop();
void audio_player_pause();
void audio_player_resume();

// Obtener información actual
player_info_t audio_player_get_info();

// Verificar estado
bool audio_player_is_playing();

// Proceso continuo (llamar en loop principal)
void audio_player_process();

#endif // AUDIO_PLAYER_H