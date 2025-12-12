/**
 * @file lcd.h
 * @brief Interfaz de alto nivel para control de una pantalla LCD 16x2 mediante I2C.
 *
 * Proporciona funciones para inicialización, escritura de texto,
 * control de cursor y actualización del estado de instrumentos.
 */

/**
 * @brief Inicializa la pantalla LCD y la interfaz I2C.
 *
 * Configura el bus I2C y realiza la secuencia estándar
 * de arranque en modo de 4 bits.
 */
void lcd_init();

/**
 * @brief Limpia completamente la pantalla LCD.
 *
 * Ejecuta el comando de borrado y espera el tiempo requerido
 * por el controlador interno.
 */
void lcd_clear();

/**
 * @brief Coloca el cursor en una posición específica.
 *
 * @param col Columna (0 a 15).
 * @param row Fila (0 o 1).
 */
void lcd_set_cursor(uint8_t col, uint8_t row);

/**
 * @brief Escribe un solo carácter en la posición actual del cursor.
 *
 * @param c Carácter ASCII a escribir.
 */
void lcd_write(char c);

/**
 * @brief Imprime una cadena completa hasta encontrar el terminador nulo.
 *
 * @param s Cadena de texto a mostrar.
 */
void lcd_print(const char *s);

/**
 * @brief Muestra en la LCD el estado actual de los instrumentos seleccionados.
 *
 * Presenta dos líneas:
 *  - Línea superior: instrumento del slot horizontal.
 *  - Línea inferior: instrumento del slot vertical.
 */
void lcd_mostrar_estado();
