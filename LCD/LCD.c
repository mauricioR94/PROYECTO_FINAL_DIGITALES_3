#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>

#include "lcd.h"
#include "botones.h"
#include "sistema.h"
#include "instrumentos.h"


uint8_t slot_activo = SLOT_H;          // 0 = Horizontal, 1 = Vertical
uint8_t instrumento_slot[2] = {0, 0};
bool sistema_activo = false;
bool mostrar_alarma = false;

static uint32_t t_calibracion;
static const uint32_t intervalo_ms = 5 * 60 * 1000; // 5 min

// BOTONES - implementación robusta con debounce por boton

static uint32_t last_time[3] = {0, 0, 0};
static bool prev_state[3] = {false, false, false};
static boton_evento_t ultimo_evento = BTN_NONE;

static bool leer_pin(uint pin) {
    // botones activos en LOW (pulldown internos no: usamos pull-up y botones a GND)
    return !gpio_get(pin);
}

void botones_init() {
    gpio_init(BTN_SLOT);
    gpio_init(BTN_NEXT);
    gpio_init(BTN_PREV);

    gpio_set_dir(BTN_SLOT, GPIO_IN);
    gpio_set_dir(BTN_NEXT, GPIO_IN);
    gpio_set_dir(BTN_PREV, GPIO_IN);

    gpio_pull_up(BTN_SLOT);
    gpio_pull_up(BTN_NEXT);
    gpio_pull_up(BTN_PREV);

    prev_state[0] = prev_state[1] = prev_state[2] = false;
    last_time[0] = last_time[1] = last_time[2] = 0;
    ultimo_evento = BTN_NONE;
}

// Mapea índice 0..2 a pines
static inline int pin_of_index(int i) {
    if (i == 0) return BTN_SLOT;
    if (i == 1) return BTN_NEXT;
    return BTN_PREV;
}

// Debounce por botón y detección de flanco de subida (pulsación)
// Se coloca evento en ultimo_evento (se consume con botones_get_evento)
void botones_update() {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    const uint32_t DEBOUNCE_MS = 180;

    // revisa cada botón
    for (int i = 0; i < 3; ++i) {
        bool curr = leer_pin(pin_of_index(i));

        // flanco: estaba release (false) y ahora pressed (true)
        if (curr && !prev_state[i]) {
            if (now - last_time[i] > DEBOUNCE_MS) {
                // registrar evento basado en i
                if (i == 0) ultimo_evento = BTN_SLOT_EVT;
                else if (i == 1) ultimo_evento = BTN_NEXT_EVT;
                else ultimo_evento = BTN_PREV_EVT;

                last_time[i] = now;
            }
        }

        // actualizar estado previo
        prev_state[i] = curr;
    }
}

// Devuelve y consume el evento disponible
boton_evento_t botones_get_evento() {
    boton_evento_t e = ultimo_evento;
    ultimo_evento = BTN_NONE;
    return e;
}

// -----------------------
// LCD - bajo nivel I2C (compatible con PCF8574 & HD44780 4-bit ext)
// -----------------------
#define I2C_PORT i2c1
#define SDA_PIN 18
#define SCL_PIN 19
#define LCD_ADDR 0x27

static void lcd_send_raw(uint8_t data) {
    // envía un byte al PCF8574 (sin usar toggles complejos aquí)
    i2c_write_blocking(I2C_PORT, LCD_ADDR, &data, 1, false);
}

static void lcd_pulse(uint8_t data) {
    // Enable = 0x04, Backlight = 0x08
    const uint8_t EN = 0x04;
    const uint8_t BL = 0x08;

    uint8_t a = data | EN | BL;
    uint8_t b = data | BL;

    lcd_send_raw(a);
    sleep_us(300);
    lcd_send_raw(b);
    sleep_us(50);
}

static void lcd_write4(uint8_t nibble, uint8_t mode) {
    // mode: 0 = cmd, 1 = data (RS)
    uint8_t rs = mode ? 0x01 : 0x00;
    uint8_t data = (nibble & 0xF0) | rs;
    lcd_pulse(data);
}

static void lcd_send(uint8_t val, uint8_t mode) {
    // high nibble then low nibble
    lcd_write4(val & 0xF0, mode);
    lcd_write4((val << 4) & 0xF0, mode);
}

void lcd_write(char c) {
    lcd_send((uint8_t)c, 1);
}

void lcd_print(const char *s) {
    while (*s) lcd_write(*s++);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    static const uint8_t off[] = {0x00, 0x40};
    lcd_send(0x80 | (col + off[row]), 0);
}

void lcd_clear() {
    lcd_send(0x01, 0);
    sleep_ms(2);
}

void lcd_init() {
    i2c_init(I2C_PORT, 100000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(40);

    // Init sequence
    lcd_send(0x33, 0);
    lcd_send(0x32, 0);
    lcd_send(0x28, 0);
    lcd_send(0x0C, 0);
    lcd_send(0x06, 0);
    lcd_clear();
}

// ----------------------------------
// UI / estado en LCD
// ----------------------------------
void lcd_mostrar_estado() {
    lcd_clear();

    lcd_set_cursor(0, 0);
    lcd_print(slot_activo == SLOT_H ? "Posicion: Hor" : "Posicion: Ver");

    // Mostrar nombre del instrumento (no numero)
    const char* nombre = instrumentos[instrumento_slot[slot_activo]];

    // Asegurarse de que no exceda 16 chars (truncar si hace falta)
    char buf[17];
    int i;
    for (i = 0; i < 16 && nombre[i]; ++i) buf[i] = nombre[i];
    buf[i] = '\0';

    lcd_set_cursor(0, 1);
    lcd_print("Inst: ");
    lcd_print(buf);
}

// -----------------------
// SISTEMA (lógica principal)
// -----------------------
void sistema_init() {
    t_calibracion = to_ms_since_boot(get_absolute_time());
}

void sistema_update() {
    boton_evento_t ev = botones_get_evento();

    switch (ev) {
        case BTN_SLOT_EVT:
            slot_activo ^= 1;
            lcd_mostrar_estado();
            break;

        case BTN_NEXT_EVT:
            instrumento_slot[slot_activo] = (instrumento_slot[slot_activo] + 1) % TOTAL_INSTRUMENTOS;
            lcd_mostrar_estado();
            break;

        case BTN_PREV_EVT:
            instrumento_slot[slot_activo] = (instrumento_slot[slot_activo] == 0) ? (TOTAL_INSTRUMENTOS - 1) : (instrumento_slot[slot_activo] - 1);
            lcd_mostrar_estado();
            break;

        default:
            break;
    }

    // Timer calibración (no bloqueante)
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - t_calibracion >= intervalo_ms) {
        mostrar_alarma = true;
        // mostrar mensaje de calibración (sobrescribe la línea inferior)
        lcd_set_cursor(0, 1);
        lcd_print("Calibrar IMU    ");
        // NOTA: la alarma se reinicia cuando se presiona cualquier botón (ver abajo)
    }

    // Si hay alarma y se presiona cualquier botón -> reset timmer y quita alarma
    // (leer los pines directamente rápido)
    if (mostrar_alarma) {
        if (leer_pin(BTN_SLOT) || leer_pin(BTN_NEXT) || leer_pin(BTN_PREV)) {
            mostrar_alarma = false;
            t_calibracion = now;
            lcd_mostrar_estado();
            // consumir evento para que no haga doble acción en la misma pulsación
            // (ya que botones_update habrá generado un evento)
            // nota: botones_get_evento() lo limpia cuando se llame en el loop principal
        }
    }
}

// -----------------------
// MAIN
// -----------------------
int main() {
    stdio_init_all();

    lcd_init();
    botones_init();
    sistema_init();

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Sistema listo");
    lcd_set_cursor(1, 0);
    lcd_print("Presione boton");

    while (1) {
        botones_update();
        sistema_update();
        sleep_ms(10); // ciclo ligero, no bloqueante
    }
    return 0;
}
