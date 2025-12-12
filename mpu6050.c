/**
 * @file mpu6050.c
 * @brief Implementación de lectura y análisis del sensor MPU6050.
 *
 * Contiene funciones de bajo nivel para comunicación I2C,
 * decodificación de registros y cálculo de orientación.
 */

#include "mpu6050.h"
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/** @brief Instancia I2C usada internamente. */
static i2c_inst_t *mpu_i2c;

/**
 * @brief Inicializa comunicación I2C y saca el MPU6050 del modo sleep.
 */
void mpu6050_init(i2c_inst_t *i2c, uint sda, uint scl) {
    mpu_i2c = i2c;

    i2c_init(i2c, 400000);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);

    uint8_t wake[2] = {0x6B, 0x00};
    i2c_write_blocking(i2c, MPU6050_ADDR, wake, 2, false);
}

/**
 * @brief Lee 14 bytes desde el registro 0x3B y decodifica acelerómetro y giroscopio.
 */
void mpu6050_read_raw(mpu6050_raw_t *data) {
    uint8_t reg = 0x3B;
    uint8_t buffer[14];

    i2c_write_blocking(mpu_i2c, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(mpu_i2c, MPU6050_ADDR, buffer, 14, false);

    data->ax = (buffer[0] << 8) | buffer[1];
    data->ay = (buffer[2] << 8) | buffer[3];
    data->az = (buffer[4] << 8) | buffer[5];
    data->gx = (buffer[8] << 8) | buffer[9];
    data->gy = (buffer[10] << 8) | buffer[11];
    data->gz = (buffer[12] << 8) | buffer[13];
}

/**
 * @brief Convierte lectura cruda de acelerómetro a "g".
 */
float mpu6050_calc_g(int16_t raw) {
    return raw / 16384.0f;
}

/**
 * @brief Calcula el ángulo de pitch usando aceleraciones normalizadas.
 */
float calc_pitch(float ax, float ay, float az) {
    return atan2f(-ax, sqrtf(ay * ay + az * az)) * (180.0f / M_PI);
}

/**
 * @brief Calcula el ángulo de roll.
 */
float calc_roll(float ay, float az) {
    return atan2f(ay, az) * (180.0f / M_PI);
}

/**
 * @brief Considera orientación vertical si pitch o roll superan 60 grados.
 */
bool is_vertical(float pitch, float roll) {
    return (fabs(pitch) > 60.0f) || (fabs(roll) > 60.0f);
}

/**
 * @brief Considera orientación horizontal si pitch y roll son menores a 30 grados.
 */
bool is_horizontal(float pitch, float roll) {
    return (fabs(pitch) < 30.0f) && (fabs(roll) < 30.0f);
}

/**
 * @brief Detecta movimiento rápido en el eje X del giroscopio.
 */
bool movement_pitch(int16_t gx) {
    return abs(gx) > 300;
}

/**
 * @brief Detecta movimiento rápido en el eje Y del giroscopio.
 */
bool movement_roll(int16_t gy) {
    return abs(gy) > 300;
}
