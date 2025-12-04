#ifndef LCD_H
#define LCD_H

void lcd_init();
void lcd_set_cursor(int col, int row);
void lcd_print(const char *s);
void lcd_clear();

// UI helpers
void lcd_show_status(const char *l1, const char *l2);
void lcd_show_calibration_prompt(void);
void lcd_show_instrument_pair(const char *v, const char *h);
void lcd_show_menu_item(const char *title, const char *item, int idx, int total);

#endif
