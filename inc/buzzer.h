#ifndef BUZZER_H
#define BUZZER_H

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/time.h"

// Inicializa o buzzer no pino especificado
void buzzer_init(uint gpio_pin);

// Toca um som curto de início (ex: 2000 Hz por 100 ms)
void buzzer_inicio(void);

// Toca um som de vitória (ex: 1500 Hz por 200 ms)
void buzzer_vitoria(void);

// Toca uma musiquinha final (ex: 1000 Hz por 500 ms)
void buzzer_musiquinha(void);

#endif // BUZZER_H