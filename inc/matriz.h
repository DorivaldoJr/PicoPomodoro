#ifndef MATRIZ_H
#define MATRIZ_H

#include "hardware/pio.h"  // Inclua outros headers se necessário

// Declaração do array com os números 0-9
extern double numeros[10][25];

// Protótipos das funções de animação e controle da matriz
uint32_t matrix_rgb(double r, double g, double b);
void animacao_inicio(PIO pio, uint sm);
void atualiza_display(int numero, PIO pio, uint sm);

#endif // MATRIZ_H