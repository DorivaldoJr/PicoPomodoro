#include "matriz.h"
#include "pico/stdlib.h" // Se precisar de funções como sleep_ms()


// Array com a representação dos números 0-9 (25 elementos por número)
double numeros[10][25] = {
    {0,1,1,1,0, 
     0,1,0,1,0,
     0,1,0,1,0,
     0,1,0,1,0,
     0,1,1,1,0}, // 0 

    {0,0,1,0,0,
     0,0,1,0,0,
     0,0,1,0,0,
     0,1,1,0,0,
     0,0,1,0,0}, // 1

    {0,1,1,1,0,
     0,1,0,0,0, 
     0,1,1,1,0, 
     0,0,0,1,0, 
     0,1,1,1,0}, // 2

    {0,1,1,1,0,
     0,0,0,1,0, 
     0,1,1,1,0, 
     0,0,0,1,0, 
     0,1,1,1,0}, // 3

    {0,1,0,0,0,
     0,0,0,1,0, 
     0,1,1,1,0, 
     0,1,0,1,0, 
     0,1,0,1,0}, // 4

    {0,1,1,1,0,
     0,0,0,1,0, 
     0,1,1,1,0, 
     0,1,0,0,0, 
     0,1,1,1,0}, // 5

    {0,1,1,1,0,
     0,1,0,1,0, 
     0,1,1,1,0, 
     0,1,0,0,0, 
     0,1,1,1,0}, // 6

    {0,0,0,0,1,
     0,1,0,0,0, 
     0,0,1,0,0, 
     0,0,0,1,0, 
     1,1,1,1,1}, // 7

    {0,1,1,1,0,
     0,1,0,1,0, 
     0,1,1,1,0, 
     0,1,0,1,0, 
     0,1,1,1,0}, // 8 

    {0,1,1,1,0,
     0,0,0,1,0, 
     0,1,1,1,0, 
     0,1,0,1,0, 
     0,1,1,1,0}  // 9
};

// Função que converte valores de cor em um inteiro para a matriz de LEDs
uint32_t matrix_rgb(double r, double g, double b) {
    unsigned char R = r * 255;
    unsigned char G = g * 255;
    unsigned char B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

void animacao_inicio(PIO pio, uint sm) {
    // Animação em um vetor único: 6 frames, cada um com 25 pixels (5x5)
    // Os valores: 0 = LED apagado, 1 = LED aceso.
    int animacao[8 * 25] = {
        // Frame 0: Apenas o centro (índice 12)
         0,0,0,0,0,
         0,0,0,0,0,
         0,0,1,0,0,
         0,0,0,0,0,
         0,0,0,0,0,
         
        // Frame 1: Centro + direções cardeais (acende acima, abaixo, esquerda e direita)
         0,0,0,0,0,
         0,0,1,0,0,
         0,1,1,1,0,
         0,0,1,0,0,
         0,0,0,0,0,
         
        // Frame 2: Bloco 3x3 central
         0,0,0,0,0,
         0,1,1,1,0,
         0,1,1,1,0,
         0,1,1,1,0,
         0,0,0,0,0,
         
        // Frame 3: Explosão total – todos os LEDs acesos (5x5)
         1,1,1,1,1,
         1,1,1,1,1,
         1,1,1,1,1,
         1,1,1,1,1,
         1,1,1,1,1,
         
        // Frame 4: Contração – volta ao bloco 3x3 central
         0,0,0,0,0,
         0,1,1,1,0,
         0,1,1,1,0,
         0,1,1,1,0,
         0,0,0,0,0,
         
        // Frame 5: Carinha feliz
         0,1,1,1,0,   
         1,0,0,0,1,   
         0,0,1,0,0,   
         0,0,0,0,0,  
         0,1,0,1,0,   

         // Frame 6: Carinha feliz piscando
         0,1,1,1,0,   
         1,0,0,0,1,   
         0,0,1,0,0,   
         0,0,0,0,0,
         0,0,0,1,0,

         // Frame 7: Carinha feliz
         0,1,1,1,0,   
         1,0,0,0,1,   
         0,0,1,0,0,   
         0,0,0,0,0,  
         0,1,0,1,0, 
    };

    // Define a cor verde com metade da intensidade
    uint32_t led_on  = matrix_rgb(0.0, 0.3, 0.0);
    uint32_t led_off = matrix_rgb(0.0, 0.0, 0.0);

    int num_frames = 8;
    int pixels_per_frame = 25;

    // Percorre cada frame da animação
    for (int f = 0; f < num_frames ; f++) {
        for (int i = 0; i < pixels_per_frame; i++) {
            int pixel = animacao[f * pixels_per_frame + i];
            if (pixel)
                pio_sm_put_blocking(pio, sm, led_on);
            else
                pio_sm_put_blocking(pio, sm, led_off);
        }
        sleep_ms(200);  // Pausa entre frames (ajuste se necessário)
        
    }

    // Mantém a carinha feliz acesa por 1 segundo
    sleep_ms(1000);
    for (int j = 0; j < pixels_per_frame; j++) {
    pio_sm_put_blocking(pio, sm, led_off);
}
}
// Função para atualizar a matriz de LEDs com um número (cor vermelha, por exemplo)
void atualiza_display(int numero, PIO pio, uint sm) {
    uint32_t valor_led;
    for (int i = 0; i < 25; i++) {
        valor_led = matrix_rgb(numeros[numero][i], 0.0, 0.0); // Assume cor vermelha 
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}


