/**
 * @file audio_player.c
 * @brief Implementación del reproductor de audio WAV con doble buffer e I2S.
 * 
 * Este módulo gestiona:
 *  - Lectura del archivo WAV desde la SD usando FatFS.
 *  - Doble buffer para reproducción continua.
 *  - Control de estados (PLAY, PAUSE, STOP).
 *  - Decodificación simple de WAV PCM 16 bits.
 * 
 * @authors
 *  - Mauricio Reyes Rosero
 *  - Reinaldo Marín Nieto
 *  - Daniel Pérez Gallego
 *  - Jorge Arroyo Niño
 */

#include "audio_player.h"
#include "i2s_output.h"
#include "sd_manager.h"
#include "ff.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>


#define AUDIO_VOLUME_SHIFT  3   

// Double buffering
static uint8_t audio_buffer_0[AUDIO_BUFFER_SIZE] __attribute__((aligned(4)));
static uint8_t audio_buffer_1[AUDIO_BUFFER_SIZE] __attribute__((aligned(4)));

static uint8_t *current_buffer = audio_buffer_0;
static uint8_t *next_buffer    = audio_buffer_1;
static uint32_t buffer_position    = 0;
static uint32_t buffer_size        = 0;
static uint32_t next_buffer_size   = 0;
static bool     need_load_next_buf = false;

// Estado del reproductor
static player_state_t player_state = PLAYER_IDLE;
static FIL      audio_file;
static bool     file_open          = false;
static uint32_t bytes_played       = 0;  // bytes enviados a I2S
static uint32_t total_bytes        = 0;  // tamaño del chunk data
static uint32_t data_start_position = 0;

// bytes del chunk data leído de la SD
static uint32_t data_bytes_read = 0;

// Información del WAV
static uint32_t wav_sample_rate = 0;
static uint16_t wav_channels    = 0;
static uint16_t wav_bits        = 0;



/** 
 * @brief Cabecera RIFF del archivo WAV.
 */
typedef struct {
    char     riff[4];
    uint32_t file_size;
    char     wave[4];
} wav_riff_header_t;

/**
 * @brief Cabecera genérica de chunk WAV.
 */
typedef struct {
    char     chunk_id[4];
    uint32_t chunk_size;
} wav_chunk_header_t;

/**
 * @brief Datos del chunk "fmt " del archivo WAV.
 */
typedef struct {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} wav_fmt_data_t;


/**
 * @brief Busca el chunk "data" dentro del archivo WAV.
 * @param file Puntero al archivo.
 * @param data_size Devuelve el tamaño del chunk.
 * @param data_position Devuelve el offset donde comienza el audio.
 * @return true si se encontró correctamente.
 */
static bool find_data_chunk(FIL *file, uint32_t *data_size, uint32_t *data_position) {
    wav_chunk_header_t chunk;
    UINT bytes_read;
    FRESULT fr;

    while (true) {
        fr = f_read(file, &chunk, sizeof(wav_chunk_header_t), &bytes_read);
        if (fr != FR_OK || bytes_read != sizeof(wav_chunk_header_t)) {
            return false;
        }

        if (memcmp(chunk.chunk_id, "data", 4) == 0) {
            *data_size     = chunk.chunk_size;
            *data_position = f_tell(file);
            printf("   Chunk 'data' en offset %lu, tamaño: %lu bytes\n",
                   *data_position, *data_size);
            return true;
        }

        fr = f_lseek(file, f_tell(file) + chunk.chunk_size);
        if (fr != FR_OK) {
            return false;
        }
    }
}


// API

bool audio_player_init() {
    printf("Iniciando reproductor de audio.\n");
    player_state = PLAYER_IDLE;
    file_open    = false;
    bytes_played = 0;
    total_bytes  = 0;
    return true;
}

bool audio_player_play(const char *filename) {
    if (player_state == PLAYER_PLAYING) {
        printf("Ya hay un archivo reproduciéndose\n");
        return false;
    }

    if (!sd_manager_is_ready()) {
        printf("SD no está lista\n");
        return false;
    }

    player_state = PLAYER_LOADING;
    printf("Cargando %s\n", filename);

    FRESULT fr;
    UINT    bytes_read;

    fr = f_open(&audio_file, filename, FA_READ);
    if (fr != FR_OK) {
        printf("Error al abrir archivo: %d\n", fr);
        player_state = PLAYER_ERROR;
        return false;
    }
    file_open = true;

    // Leer RIFF header
    wav_riff_header_t riff;
    fr = f_read(&audio_file, &riff, sizeof(wav_riff_header_t), &bytes_read);
    if (fr != FR_OK || bytes_read != sizeof(wav_riff_header_t)) {
        printf("Error al leer RIFF\n");
        f_close(&audio_file);
        file_open    = false;
        player_state = PLAYER_ERROR;
        return false;
    }

    if (memcmp(riff.riff, "RIFF", 4) != 0 ||
        memcmp(riff.wave, "WAVE", 4) != 0) {
        printf("Archivo no es WAV válido\n");
        f_close(&audio_file);
        file_open    = false;
        player_state = PLAYER_ERROR;
        return false;
    }

    // Buscar chunk 'fmt '
    wav_chunk_header_t chunk;
    wav_fmt_data_t     fmt;
    bool               fmt_found = false;

    while (true) {
        fr = f_read(&audio_file, &chunk, sizeof(wav_chunk_header_t), &bytes_read);
        if (fr != FR_OK || bytes_read != sizeof(wav_chunk_header_t)) {
            break;
        }

        if (memcmp(chunk.chunk_id, "fmt ", 4) == 0) {
            uint32_t fmt_size = (chunk.chunk_size < sizeof(wav_fmt_data_t))
                                ? chunk.chunk_size
                                : sizeof(wav_fmt_data_t);

            fr = f_read(&audio_file, &fmt, fmt_size, &bytes_read);
            if (fr == FR_OK) {
                fmt_found = true;

                if (chunk.chunk_size > fmt_size) {
                    f_lseek(&audio_file,
                            f_tell(&audio_file) + (chunk.chunk_size - fmt_size));
                }
            }
            break;
        }

        // No era 'fmt ', saltar contenido
        fr = f_lseek(&audio_file, f_tell(&audio_file) + chunk.chunk_size);
        if (fr != FR_OK) {
            break;
        }
    }

    if (!fmt_found) {
        printf("No se encontró chunk 'fmt'\n");
        f_close(&audio_file);
        file_open    = false;
        player_state = PLAYER_ERROR;
        return false;
    }

    // Validar formato
    if (fmt.audio_format != 1) {
        printf("Formato no PCM (%u)\n", fmt.audio_format);
        f_close(&audio_file);
        file_open    = false;
        player_state = PLAYER_ERROR;
        return false;
    }

    if (fmt.bits_per_sample != 16) {
        printf("Solo 16 bits soportados (archivo: %u bits)\n",
               fmt.bits_per_sample);
        f_close(&audio_file);
        file_open    = false;
        player_state = PLAYER_ERROR;
        return false;
    }

    if (fmt.num_channels != 1 && fmt.num_channels != 2) {
        printf("Solo 1 o 2 canales (archivo: %u)\n", fmt.num_channels);
        f_close(&audio_file);
        file_open    = false;
        player_state = PLAYER_ERROR;
        return false;
    }

    // Buscar chunk 'data'
    uint32_t data_size = 0;
    if (!find_data_chunk(&audio_file, &data_size, &data_start_position)) {
        printf(" No se encontró chunk 'data'\n");
        f_close(&audio_file);
        file_open    = false;
        player_state = PLAYER_ERROR;
        return false;
    }

    printf("WAV válido:\n");
    printf("   Sample rate: %lu Hz\n", fmt.sample_rate);
    printf("   Canales:     %u\n", fmt.num_channels);
    printf("   Bits:        %u\n", fmt.bits_per_sample);
    printf("   Data size:   %lu bytes (%.2f KB)\n",
           data_size, data_size / 1024.0f);
    printf("   Duración:    %.2f s\n",
           (float)data_size /
           (fmt.sample_rate * fmt.num_channels * (fmt.bits_per_sample / 8)));

    wav_sample_rate = fmt.sample_rate;
    wav_channels    = fmt.num_channels;
    wav_bits        = fmt.bits_per_sample;
    total_bytes     = data_size;
    bytes_played    = 0;
    data_bytes_read = 0;

    // Preparar buffers
    current_buffer     = audio_buffer_0;
    next_buffer        = audio_buffer_1;
    buffer_position    = 0;
    buffer_size        = 0;
    next_buffer_size   = 0;
    need_load_next_buf = false;

    // Reconfigurar I2S al sample rate del archivo
    i2s_output_stop();
    if (!i2s_output_init(wav_sample_rate)) {
        printf("Error al reinicializar I2S\n");
        f_close(&audio_file);
        file_open    = false;
        player_state = PLAYER_ERROR;
        return false;
    }

    // Cargar primer buffer
    uint32_t bytes_to_read = (total_bytes > AUDIO_BUFFER_SIZE)
                             ? AUDIO_BUFFER_SIZE
                             : total_bytes;
    if (bytes_to_read > 0) {
        fr = f_read(&audio_file, current_buffer, bytes_to_read, &bytes_read);
        if (fr != FR_OK || bytes_read == 0) {
            printf("Error al leer datos (buffer 0)\n");
            f_close(&audio_file);
            file_open    = false;
            player_state = PLAYER_ERROR;
            return false;
        }
        buffer_size     = bytes_read;
        buffer_position = 0;
        data_bytes_read += bytes_read;
    }

    // Cargar segundo buffer
    bytes_to_read = total_bytes - data_bytes_read;
    if (bytes_to_read > AUDIO_BUFFER_SIZE) {
        bytes_to_read = AUDIO_BUFFER_SIZE;
    }

    if (bytes_to_read > 0) {
        fr = f_read(&audio_file, next_buffer, bytes_to_read, &bytes_read);
        if (fr != FR_OK) {
            printf("Error al leer datos (buffer 1)\n");
            f_close(&audio_file);
            file_open    = false;
            player_state = PLAYER_ERROR;
            return false;
        }
        next_buffer_size = bytes_read;
        data_bytes_read += bytes_read;
    } else {
        next_buffer_size = 0;
    }

    need_load_next_buf = false;
    player_state       = PLAYER_PLAYING;

    printf("Reproduciendo. (%lu bytes)\n", total_bytes);
    return true;
}

void audio_player_stop() {
    if (player_state != PLAYER_PLAYING && player_state != PLAYER_PAUSED) {
        return;
    }

    if (file_open) {
        f_close(&audio_file);
        file_open = false;
    }

    player_state    = PLAYER_IDLE;
    bytes_played    = 0;
    buffer_position = 0;
    buffer_size     = 0;
    next_buffer_size = 0;

    printf("Reproducción detenida\n");
}

void audio_player_pause() {
    if (player_state == PLAYER_PLAYING) {
        player_state = PLAYER_PAUSED;
        printf("Pausado\n");
    }
}

void audio_player_resume() {
    if (player_state == PLAYER_PAUSED) {
        player_state = PLAYER_PLAYING;
        printf("Reanudado\n");
    }
}

void audio_player_process() {
    if (player_state != PLAYER_PLAYING) {
        return;
    }

    if (bytes_played >= total_bytes) {
        audio_player_stop();
        printf("Reproducción completada (%lu/%lu bytes)\n",
               bytes_played, total_bytes);
        return;
    }

    if (!i2s_output_can_send()) {
        return;
    }

    // Se acabó el buffer?
    if (buffer_position >= buffer_size) {
        if (next_buffer_size == 0) {
            audio_player_stop();
            printf("Reproducción completada (%lu/%lu bytes)\n",
                   bytes_played, total_bytes);
            return;
        }

        // Intercambiar buffers
        uint8_t *tmp = current_buffer;
        current_buffer   = next_buffer;
        next_buffer      = tmp;
        buffer_size      = next_buffer_size;
        buffer_position  = 0;
        need_load_next_buf = true;
    }

    // Cargar próximo buffer desde SD si hace falta
    if (need_load_next_buf) {
        UINT bytes_read;
        uint32_t bytes_left = (total_bytes > data_bytes_read)
                              ? (total_bytes - data_bytes_read)
                              : 0;

        uint32_t bytes_to_read = (bytes_left > AUDIO_BUFFER_SIZE)
                                 ? AUDIO_BUFFER_SIZE
                                 : bytes_left;

        if (bytes_to_read > 0) {
            FRESULT fr = f_read(&audio_file, next_buffer, bytes_to_read, &bytes_read);
            if (fr != FR_OK) {
                printf("Error al recargar buffer desde SD\n");
                audio_player_stop();
                return;
            }
            next_buffer_size = bytes_read;
            data_bytes_read += bytes_read;
        } else {
            next_buffer_size = 0;
        }

        need_load_next_buf = false;
    }

    if (wav_bits == 16) {
        int16_t left  = 0;
        int16_t right = 0;

        if (wav_channels == 2) {
            if (buffer_position + 4 > buffer_size) {
                return;
            }

            left  = (int16_t)( current_buffer[buffer_position] |
                              (current_buffer[buffer_position + 1] << 8));
            right = (int16_t)( current_buffer[buffer_position + 2] |
                              (current_buffer[buffer_position + 3] << 8));

            buffer_position += 4;
            bytes_played    += 4;
        } else if (wav_channels == 1) {
            if (buffer_position + 2 > buffer_size) {
                return;
            }

            left = (int16_t)( current_buffer[buffer_position] |
                             (current_buffer[buffer_position + 1] << 8));
            right = left;

            buffer_position += 2;
            bytes_played    += 2;
        } else {
            return;
        }


        // APLICAR VOLUMEN

        if (AUDIO_VOLUME_SHIFT > 0) {
            left  = (int16_t)( ((int32_t)left)  >> AUDIO_VOLUME_SHIFT );
            right = (int16_t)( ((int32_t)right) >> AUDIO_VOLUME_SHIFT );
        }

        i2s_output_send_frame(left, right);
    }
}

player_info_t audio_player_get_info() {
    player_info_t info = {
        .state            = player_state,
        .bytes_played     = bytes_played,
        .total_bytes      = total_bytes,
        .sample_rate      = wav_sample_rate,
        .num_channels     = wav_channels,
        .bits_per_sample  = wav_bits,
        .progress_percent = (total_bytes > 0)
                            ? ((float)bytes_played / (float)total_bytes * 100.0f)
                            : 0.0f
    };
    return info;
}

bool audio_player_is_playing() {
    return player_state == PLAYER_PLAYING;
}
