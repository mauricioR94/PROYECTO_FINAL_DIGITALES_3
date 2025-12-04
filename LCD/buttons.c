#include "buttons.h"
#include "hardware/gpio.h"

#define BTN_UP_PIN     14
#define BTN_DOWN_PIN   15
#define BTN_LEFT_PIN   16
#define BTN_RIGHT_PIN  17
#define BTN_SELECT_PIN 18

void buttons_init() {
    gpio_init(BTN_UP_PIN);
    gpio_set_dir(BTN_UP_PIN, false);
    gpio_pull_up(BTN_UP_PIN);

    gpio_init(BTN_DOWN_PIN);
    gpio_set_dir(BTN_DOWN_PIN, false);
    gpio_pull_up(BTN_DOWN_PIN);

    gpio_init(BTN_LEFT_PIN);
    gpio_set_dir(BTN_LEFT_PIN, false);
    gpio_pull_up(BTN_LEFT_PIN);

    gpio_init(BTN_RIGHT_PIN);
    gpio_set_dir(BTN_RIGHT_PIN, false);
    gpio_pull_up(BTN_RIGHT_PIN);

    gpio_init(BTN_SELECT_PIN);
    gpio_set_dir(BTN_SELECT_PIN, false);
    gpio_pull_up(BTN_SELECT_PIN);
}

bool buttons_up_pressed() {
    return gpio_get(BTN_UP_PIN) == 0;
}

bool buttons_down_pressed() {
    return gpio_get(BTN_DOWN_PIN) == 0;
}

bool buttons_left_pressed() {
    return gpio_get(BTN_LEFT_PIN) == 0;
}

bool buttons_right_pressed() {
    return gpio_get(BTN_RIGHT_PIN) == 0;
}

bool buttons_select_pressed() {
    return gpio_get(BTN_SELECT_PIN) == 0;
}

bool ui_check_any_button_pressed() {
    return buttons_up_pressed() || buttons_down_pressed() || buttons_left_pressed() || buttons_right_pressed() || buttons_select_pressed();
}