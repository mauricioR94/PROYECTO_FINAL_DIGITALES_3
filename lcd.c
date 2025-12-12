/**
 * @file lcd.c
 * @brief Rutinas para manejo de LCD 16x2 mediante interface I2C PCF8574.
 */

#include "lcd.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <string.h>

#include "sistema.h"
#include "instrumentos.h"

#define I2C_PORT i2c1
#define SDA_PIN  2
#define SCL_PIN  3
#define LCD_ADDR 0x27

/**
 * @brief Envía un byte crudo al LCD por I2C.
 */
static void lcd_send_raw(uint8_t data) {
    i2c_write_blocking(I2C_PORT, LCD_ADDR, &data, 1, false);
}

/**
 * @brief Genera un pulso de habilitación (EN).
 */
static void lcd_pulse(uint8_t data) {
    const uint8_t EN = 0x04;
    const uint8_t BL = 0x08;

    uint8_t a = data | EN | BL;
    uint8_t b = data | BL;

    lcd_send_raw(a);
    sleep_us(300);
    lcd_send_raw(b);
    sleep_us(50);
}

/**
 * @brief Envía un nibble de 4 bits al LCD.
 * @param nibble Parte alta del byte.
 * @param mode 1=carácter, 0=comando.
 */
static void lcd_write4(uint8_t nibble, uint8_t mode) {
    uint8_t rs = mode ? 0x01 : 0x00;
    uint8_t data = (nibble & 0xF0) | rs;
    lcd_pulse(data);
}

/**
 * @brief Envía un byte completo al LCD dividiéndolo en dos nibbles.
 */
static void lcd_send(uint8_t val, uint8_t mode) {
    lcd_write4(val & 0xF0, mode);
    lcd_write4((val << 4) & 0xF0, mode);
}

/**
 * @brief Escribe un carácter en el LCD.
 */
void lcd_write(char c) {
    lcd_send((uint8_t)c, 1);
}

/**
 * @brief Imprime una cadena completa en el LCD.
 */
void lcd_print(const char *s) {
    while (*s) lcd_write(*s++);
}

/**
 * @brief Posiciona el cursor en la LCD.
 * @param col 0..15
 * @param row 0..1
 */
void lcd_set_cursor(uint8_t col, uint8_t row) {
    static const uint8_t off[] = {0x00, 0x40};
    if (row > 1) row = 1;
    lcd_send(0x80 | (col + off[row]), 0);
}

/**
 * @brief Limpia la pantalla de la LCD.
 */
void lcd_clear() {
    lcd_send(0x01, 0);
    sleep_ms(2);
}

/**
 * @brief Inicializa la LCD por I2C en modo 4 bits.
 */
void lcd_init() {
    i2c_init(I2C_PORT, 100000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(40);

    lcd_send(0x33, 0);
    lcd_send(0x32, 0);
    lcd_send(0x28, 0);
    lcd_send(0x0C, 0);
    lcd_send(0x06, 0);
    lcd_clear();

    printf("LCD inicializada\n");
}

/**
 * @brief Muestra el estado actual de instrumentos en dos filas.
 */
void lcd_mostrar_estado() {
    lcd_clear();

    if (total_instrumentos == 0) {
        lcd_set_cursor(0, 0);
        lcd_print("Sin instrumentos");
        lcd_set_cursor(0, 1);
        lcd_print("Revisa index.txt");
        return;
    }

    uint8_t idx_h = instrumento_slot[SLOT_H];
    if (idx_h >= total_instrumentos) idx_h = 0;
    const char *nombre_h = instrumentos[idx_h];

    uint8_t idx_v = instrumento_slot[SLOT_V];
    if (idx_v >= total_instrumentos) idx_v = 0;
    const char *nombre_v = instrumentos[idx_v];

    char line[17];
    memset(line, ' ', 16);
    line[16] = '\0';

    const char *prefix1 = "Inst 1-";
    int pos = 0;
    while (prefix1[pos] && pos < 16) line[pos] = prefix1[pos], pos++;
    int i = 0;
    while (nombre_h[i] && pos < 16) line[pos++] = nombre_h[i++];

    if (slot_activo == SLOT_H) {
        line[14] = '<';
        line[15] = '-';
    }

    lcd_set_cursor(0, 0);
    lcd_print(line);

    memset(line, ' ', 16);
    line[16] = '\0';

    const char *prefix2 = "Inst 2-";
    pos = 0;
    while (prefix2[pos] && pos < 16) line[pos] = prefix2[pos], pos++;
    i = 0;
    while (nombre_v[i] && pos < 16) line[pos++] = nombre_v[i++];

    if (slot_activo == SLOT_V) {
        line[14] = '<';
        line[15] = '-';
    }

    lcd_set_cursor(0, 1);
    lcd_print(line);
}
