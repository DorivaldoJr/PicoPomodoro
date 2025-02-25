#ifndef MATRIZ_H
#define MATRIZ_H

#include "hardware/pio.h"  

extern PIO pio_global; // Variavel global para acesso ao PIO pelas interrupcoes
extern uint matrix_sm_global; // Variavel global para acesso as maquinas de estado pelas interrupcoes
extern uint led_steps;          // Contador de quantos LEDs já acenderam
extern uint led_max_steps;
extern uint32_t led_buffer[];  // Buffer para os LEDs

#define NUM_PIXELS 25 //matriz led
// Declaração do array com os números 0-9
extern double numeros[10][25];

// Protótipos das funções de animação e controle da matriz
uint32_t matrix_rgb(double r, double g, double b);
void animacao_inicio(PIO pio, uint sm);
void atualiza_display(int numero, PIO pio, uint sm);
void update_led_matrix();
void acender_led_sequencialmente();



#endif // MATRIZ_H