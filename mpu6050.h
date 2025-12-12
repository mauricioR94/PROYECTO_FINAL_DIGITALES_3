/**
 * @file mpu6050.h
 * @brief Control básico del sensor MPU6050: inicialización, lectura y análisis.
 *
 * Define estructuras, constantes y funciones públicas para obtener
 * aceleraciones, giroscopio y calcular orientación.
 */

#ifndef MPU6050_H
#define MPU6050_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdbool.h>
#include <stdint.h>

/** @brief Dirección I2C por defecto del MPU6050. */
#define MPU6050_ADDR 0x68

/**
 * @brief Estructura que contiene las lecturas crudas del MPU6050.
 *
 * ax, ay, az corresponden a acelerómetro.
 * gx, gy, gz corresponden a giroscopio.
 */
typedef struct {
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
} mpu6050_raw_t;

/**
 * @brief Inicializa el módulo MPU6050 y su interfaz I2C.
 *
 * @param i2c Instancia I2C usada.
 * @param sda Pin SDA.
 * @param scl Pin SCL.
 */
void mpu6050_init(i2c_inst_t *i2c, uint sda, uint scl);

/**
 * @brief Lee las 6 mediciones crudas de acelerómetro y giroscopio.
 *
 * @param data Puntero a la estructura donde se guardarán las lecturas.
 */
void mpu6050_read_raw(mpu6050_raw_t *data);

/**
 * @brief Convierte un valor crudo del acelerómetro a unidades de gravedad.
 *
 * @param raw Valor entero del eje leído.
 * @return Aceleración en "g".
 */
float mpu6050_calc_g(int16_t raw);

/**
 * @brief Calcula el ángulo de pitch a partir de aceleraciones.
 *
 * @param ax Aceleración en X.
 * @param ay Aceleración en Y.
 * @param az Aceleración en Z.
 * @return Pitch en grados.
 */
float calc_pitch(float ax, float ay, float az);

/**
 * @brief Calcula el ángulo de roll usando aceleraciones.
 *
 * @param ay Aceleración en Y.
 * @param az Aceleración en Z.
 * @return Roll en grados.
 */
float calc_roll(float ay, float az);

/**
 * @brief Determina si el dispositivo está en orientación vertical.
 *
 * @param pitch Ángulo de pitch.
 * @param roll  Ángulo de roll.
 * @return true si está vertical.
 */
bool is_vertical(float pitch, float roll);

/**
 * @brief Determina si el dispositivo está horizontal.
 *
 * @param pitch Ángulo de pitch.
 * @param roll  Ángulo de roll.
 * @return true si está horizontal.
 */
bool is_horizontal(float pitch, float roll);

/**
 * @brief Detecta un movimiento brusco alrededor del eje de pitch.
 *
 * @param gx Valor crudo del giroscopio en X.
 * @return true si se detecta un movimiento rápido.
 */
bool movement_pitch(int16_t gx);

/**
 * @brief Detecta un movimiento brusco alrededor del eje de roll.
 *
 * @param gy Valor crudo del giroscopio en Y.
 * @return true si se detecta un movimiento rápido.
 */
bool movement_roll(int16_t gy);

#endif
