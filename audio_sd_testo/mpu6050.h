#ifndef MPU6050_H
#define MPU6050_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdbool.h>
#include <stdint.h>

// Dirección I2C del MPU6050
#define MPU6050_ADDR 0x68

// Estructura para guardar lecturas crudas
typedef struct {
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
} mpu6050_raw_t;

// Inicialización
void mpu6050_init(i2c_inst_t *i2c, uint sda, uint scl);

// Lectura
void mpu6050_read_raw(mpu6050_raw_t *data);

// Conversión
float mpu6050_calc_g(int16_t raw);

// Cálculo de ángulos
float calc_pitch(float ax, float ay, float az);
float calc_roll(float ay, float az);

// Detección de orientación
bool is_vertical(float pitch, float roll);
bool is_horizontal(float pitch, float roll);

// Detección de movimiento
bool movement_pitch(int16_t gx);
bool movement_roll(int16_t gy);

#endif
