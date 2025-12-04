/* buttons.h */
#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>

void buttons_init();
bool buttons_up_pressed();
bool buttons_down_pressed();
bool buttons_left_pressed();
bool buttons_right_pressed();
bool buttons_select_pressed();
bool ui_check_any_button_pressed();

#endif // BUTTONS_H
