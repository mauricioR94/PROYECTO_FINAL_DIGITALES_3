# HANDino Motion Tool


**Proyecto:** Instrumento musical embebido controlado por movimiento 

**Autores:**

  - Mauricio Reyes Rosero
  - Reinaldo Marín Nieto
  - Daniel Pérez Gallego
  - Jorge Arroyo Niño



 El presente es un prototipo de instrumento portátil que reproduce sonidos y efectos en tiempo real a partir de gestos detectados por una IMU y entradas físicas.



## Tabla de contenidos
- [Descripción](#descripción)
- [Guía de uso](#Guía-de-uso)
- [GPIO usados](#gpio-usados)
- [Objetivos](#objetivos)
- [Características principales](#características-principales)
- [Marco teórico](#marco-terocio)
- [Estructura del repositorio](#estructura-del-repositorio)


## Descripción
HANDino Motion Tool es un instrumento musical embebido diseñado para reproducir samples mediante el movimiento de la mano y la pulsación de botones. 

El sistema interpreta lecturas de acelerómetro y giroscopio de una IMU MPU6050, las procesa en un microcontrolador (RP2040 / Raspberry Pi Pico) y reproduce samples por un DAC I²S/amplificador. 

Permite cargar las librerías de audio desde microSD, cambiar instrumentos y aplicar efectos de trémolo, mapeando movimientos a parámetros sonoros.

## Guía de uso
- **Encendido**: el dispositivo arranca encendiendo el interruptor, inicia la calibración IMU y carga bibliotecas desde microSD.  
- **Tocar**: pulsar botones para reproducir notas; girar el instrumento en posición vertical u horizontal para cambiar entre  los dos instrumentos seleccionados, girar en paralelo continuamente para activar efectos de trémolo  
- **Navegación UI**: El proyecto utiliza una pantalla LCD 16x2 con interfaz I2C como medio principal de visualización,  y usa tres botones dedicados para listar y seleccionar instrumentos desde la LCD, 
 - El primer botón de la izquierda funciona para establecer el instrumento anterior en la lista. 
 - El botón del medio servirá para alternar el slot de instrumento vertical u horizontal
 - Rl botón de la derecha será para el instrumento siguiente.

## Objetivos

- Integrar una IMU MPU6050 y procesar sus datos para obtener orientación, aceleración y eventos gestuales.

- Implementar un sistema de reproducción de audio mediante DAC I2S UDA1334A capaz de reproducir samples desde una tarjeta microSD con baja latencia.

- Diseñar un sistema de entrada basado en botones que permita ejecutar notas musicales.

- Implementar una interfaz LCD básica de interfaz y control para navegación y estados del sistema.

- Optimizar la arquitectura del firmware para garantizar estabilidad y asegurar el flujo continuo de audio.



## Características principales
- Detección de gestos con la IMU MPU6050 (pitch / roll / yaw, magnitud de aceleración, velocidad angular).
- Mapeo gestual configurable: cambio de instrumento y efectos de trémolo.
- Reproducción de samples desde microSD vía I²S con DMA.
- Interfaz física: pantalla LCD (I²C) + botones para navegación y selección.
- Modo de bajo consumo tras 10 minutos inactivo; reactivación rápida por botón.
- Gestión de librerías (listado / carga / selección de hasta 10 instrumentos predefinidos).




## Marco teórico

El proyecto combina tres áreas principales: adquisición de movimiento, reproducción digital de audio y sistemas embebidos en tiempo real.

#### **Unidades de Medición Inercial (IMU)**

- Acelerómetro triaxial (mide aceleración en X, Y, Z).

- Giroscopio triaxial (velocidad angular en X, Y, Z).

A partir de estas lecturas se pueden derivar:

- Pitch, roll y yaw, mediante relaciones trigonométricas.

- Eventos bruscos, útiles para activar efectos de trémolo.


#### **Audio digital e interfaz I2S**

El protocolo I2S transmite audio PCM en serie usando tres señales:

- BCLK: bit clock

- LRCK/WS: word select (indica canal izquierdo/derecho)

- DIN: datos digitales del audio

El DAC UDA1334A convierte estos datos en una señal analógica para parlantes o audífonos.
Para evitar cortes o chasquidos, el firmware debe usar: DMA para mover los datos sin bloquear la CPU y frecuencias típicas de 40 kHz, 16 bits por muestra


#### **Sistema de archivos y lectura por SPI**

La tarjeta microSD usa el bus SPI1, donde se manejan:

- MOSI, MISO, SCK y CS.

Los samples deben leerse desde la SD en bloques, por lo que se requiere un pre-buffering, lecturas secuenciales y minimizar accesos aleatorios para evitar latencia extra


## GPIO usados

| GPIO Pico | Tipo / Dirección | Función / Señal | Componente |
|-----------|------------------|------------------|-------------|
| **6**  | Entrada | Botón: Si | Botonera (notas) |
| **7**  | Entrada | Botón: La | Botonera (notas) |
| **8**  | Entrada | Botón: Sol | Botonera (notas) |
| **9**  | Entrada | Botón: Fa | Botonera (notas) |
| **18** | Entrada | Botón: Do | Botonera (notas) |
| **19** | Entrada | Botón: Mi | Botonera (notas) |
| **20** | Entrada | Botón: Re | Botonera (notas) |
| **26** | Salida | SCK (SPI1) | Módulo SD |
| **27** | Salida | MOSI / TX (SPI1) | Módulo SD |
| **28** | Entrada | MISO / RX (SPI1) | Módulo SD |
| **22** | Salida | CS del módulo SD | Módulo SD |
| **10** | Salida | BCLK (I2S) | DAC UDA1334A |
| **11** | Salida | LRCK / WS (I2S) | DAC UDA1334A |
| **12** | Salida | DIN (I2S) | DAC UDA1334A |
| **4** | Bidireccional | SDA (I2C0) | IMU MPU6050 |
| **5** | Bidireccional | SCL (I2C0) | IMU MPU6050 |
| **— 3V3** | Alimentación | VCC | SD, DAC, MPU6050 |
| **— GND** | Tierra | GND común | Todos los módulos |
| **— AD0** | Config | Dirección 0x68 | IMU MPU6050 |




## Conclusiones del Proyecto

- Un sistema embebido simple como la Raspberry Pi Pico puede manejar simultáneamente lectura de IMU, manejo de botones, lectura de microSD y transmisión I2S siempre que se priorice correctamente la tarea de audio.
- La arquitectura basada en PIO + DMA es suficiente para reproducir audio sin cortes ni artefactos sonoros, incluso cuando el sistema está leyendo archivos desde la SD.
- La integración de buses distintos (I2C, SPI, I2S) confirma que la Pico soporta varios periféricos concurrentes sin congestión perceptible cuando el firmware está ordenado y modular.
- Es importante filtrar y estabilizar sensores antes de usarlos para interacción musical

- El dispositivo es completamente viable como producto reproducible.  
  La electrónica es estándar: IMU barata, DAC I2S común, lector SD y pi pico
- Para producción masiva solo haría falta:
  - Crear un PCB dedicado que unifique SD, IMU, botones y DAC.
  - Integrar un amplificador de audio mejor, dependiendo del volumen deseado.
  - Diseñar una carcasa ergonómica impresa en 3D
- La modularidad del firmware permite añadir nuevos instrumentos o efectos sin tocar el hardware, lo cual reduce costos si el proyecto se escalara a cientos o miles de unidades.
- Desde el punto de vista industrial, la mayor limitación sería la alimentación: requeriría una batería LiPo segura e integrada con carga USB-C.



