#include "lcd.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

#define I2C_PORT i2c1
#define SDA_PIN 18
#define SCL_PIN 19
#define LCD_ADDR 0x27

#define LCD_BACKLIGHT 0x08
#define ENABLE_BIT 0x04

static void lcd_write4(uint8_t data) {
    uint8_t buf[1];

    buf[0] = data | ENABLE_BIT | LCD_BACKLIGHT;
    i2c_write_blocking(I2C_PORT, LCD_ADDR, buf, 1, false);
    sleep_us(500);

    buf[0] = (data & ~ENABLE_BIT) | LCD_BACKLIGHT;
    i2c_write_blocking(I2C_PORT, LCD_ADDR, buf, 1, false);
    sleep_us(50);
}

static void lcd_send_cmd(uint8_t cmd) {
    lcd_write4(cmd & 0xF0);
    lcd_write4((cmd << 4) & 0xF0);
}

static void lcd_send_data(uint8_t data) {
    lcd_write4((data & 0xF0) | 0x01);
    lcd_write4(((data << 4) & 0xF0) | 0x01);
}

void lcd_init() {
    sleep_ms(50);

    lcd_write4(0x30);
    sleep_ms(5);
    lcd_write4(0x30);
    sleep_us(200);
    lcd_write4(0x30);
    sleep_us(200);

    lcd_write4(0x20);

    lcd_send_cmd(0x28);
    lcd_send_cmd(0x0C);
    lcd_send_cmd(0x06);
    lcd_send_cmd(0x01);
    sleep_ms(2);
}

void lcd_clear() {
    lcd_send_cmd(0x01);
    sleep_ms(2);
}

void lcd_set_cursor(int col, int row) {
    uint8_t offsets[] = {0x00, 0x40};
    lcd_send_cmd(0x80 | (col + offsets[row]));
}

void lcd_print(const char *s) {
    while (*s) lcd_send_data(*s++);
}

void lcd_show_status(const char *l1, const char *l2) {
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print(l1);
    lcd_set_cursor(0, 1);
    lcd_print(l2);
}

void lcd_show_calibration_prompt(void) {
    lcd_show_status("Modo calib.", "Base y boton...");
}

void lcd_show_instrument_pair(const char *v, const char *h) {
    char a[17], b[17];
    snprintf(a, 17, "V:%s", v);
    snprintf(b, 17, "H:%s", h);
    lcd_show_status(a, b);
}

void lcd_show_menu_item(const char *title, const char *item, int idx, int total) {
    char top[17];
    snprintf(top, 17, "%s %d/%d", title, idx+1, total);
    lcd_show_status(top, item);
}
