#include <stdio.h>
#include "pico/stdlib.h"
#include "inc/font.h"
#include "inc/ssd1306.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "inc/matriz.h"
#include "hardware/pio.h"
#include "pio_matrix.pio.h"
#include "hardware/adc.h"

// Macros I2c
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
// Macros Matriz led's
#define NUM_PIXELS 25 //matriz led
#define OUT_PIN 7 // matriz led
// Macros botoes e led RGB
#define BUTTON_JOY 22
#define BUTTON_A 5
#define BUTTON_B 6
#define LED_RED 13
#define LED_GREEN 11

//Definicoes de pinos para leitura do joystick
#define VRX_PIN 26
#define VRY_PIN 27
uint comando = 0;
bool leitura = false;
uint codigo_tempo = 0;
uint tempo=0;
#define TIME_25_MIN_US_ESTUDO 1500000000ULL
#define TIME_30_MIN_US_ESTUDO 1800000000ULL
#define TIME_35_MIN_US_ESTUDO 2100000000ULL
#define TIME_PAUSA_CURTA 
#define TIME_PAUSA_LONGA


ssd1306_t ssd; // Inicializa a estrutura do display, ficou aqui para nao dar erro de compilacao


void setup_inicialicacao_softaware (PIO pio, uint sm){ // Inicializa o softaware com as boas vindas e instrucoes para o usuario 
    // Inicializcao serial
    printf("Bem vindo ao PicoPomodoro!\n"); 
    // Inicializacao via Display
    ssd1306_draw_string(&ssd, "BEM VINDO AO ", 15, 0);
    ssd1306_draw_string(&ssd, "PICO POMODORO! ", 12, 12);
    ssd1306_draw_string(&ssd, "Pressione o", 0, 32);
    ssd1306_draw_string(&ssd, "joystick para", 0 , 42);
    ssd1306_draw_string(&ssd, "configurar", 0 , 52);
    ssd1306_send_data(&ssd);
   // Inicializacao pela matriz de leds
    animacao_inicio(pio, sm);
}

// Tempos em microsegundos (para estudo e pausas)
#define TIME_25_MIN_US_ESTUDO 1500000000ULL  // 25 minutos
#define TIME_30_MIN_US_ESTUDO 1800000000ULL  // 30 minutos
#define TIME_35_MIN_US_ESTUDO 2100000000ULL  // 35 minutos
#define TIME_PAUSA_CURTA_US   (5ULL * 60ULL * 1000000ULL)   // 5 minutos

// ---------------------------
// Variáveis Globais
// ---------------------------
ssd1306_t ssd;  // Estrutura do display (global para facilitar as chamadas do display)
uint64_t tempo_estudo_config = 0;   // Valor final em microsegundos para estudo
uint64_t tempo_descanso_config = 0; // Valor final em microsegundos para pausa curta

void atualiza_exibicao_selecao(const char *titulo, const char *opcao) {
    char buf[32];
    ssd1306_fill(&ssd, false);
    sprintf(buf, "%s", titulo);
    ssd1306_draw_string(&ssd, buf, 0, 0);
    sprintf(buf, "%s", opcao);
    ssd1306_draw_string(&ssd, buf, 0, 10);
    ssd1306_draw_string(&ssd, "Use eixo Y", 0, 20);
    ssd1306_draw_string(&ssd, "Pressione Btn", 0, 30);
    ssd1306_send_data(&ssd);
}
void configurar_tempos(void) {
    char buf[32];  // Buffer para sprintf

    // Bloco 1: Configurar tempo de estudo
    uint indice_estudo = 1;  // 1 => 25 min, 2 => 30 min, 3 => 35 min
    const uint64_t opcoes_estudo[3] = { TIME_25_MIN_US_ESTUDO, TIME_30_MIN_US_ESTUDO, TIME_35_MIN_US_ESTUDO };
    const char *texto_estudo[3] = { "25 min", "30 min", "35 min" };

    atualiza_exibicao_selecao("Estudo:", texto_estudo[indice_estudo - 1]);
    
    bool esperando_neutro = false;
    while (true) {
        adc_select_input(0);  // Canal 1 para o eixo Y (GP27)
        uint16_t valor_y = adc_read();
        // (Opcional) Debug: printf("Valor Y: %u\n", valor_y);
        
        // Define faixa neutra: entre 1700 e 2700
        if (valor_y > 1700 && valor_y < 2700) {
            esperando_neutro = false;
        }
        
        if (!esperando_neutro) {
            if (valor_y >= 2700) {
                indice_estudo++;
                if (indice_estudo > 3) indice_estudo = 1;
                atualiza_exibicao_selecao("Estudo:", texto_estudo[indice_estudo - 1]);
                esperando_neutro = true;
                sleep_ms(100);
            } else if (valor_y <= 1700) {
                if (indice_estudo > 1)
                    indice_estudo--;
                else
                    indice_estudo = 3;
                atualiza_exibicao_selecao("Estudo:", texto_estudo[indice_estudo - 1]);
                esperando_neutro = true;
                sleep_ms(100);
            }
        }
        
        // Se o usuário confirmar pressionando o botão do joystick:
        if (gpio_get(BUTTON_JOY) == 0) {
            sleep_ms(300); // debounce do botão
            break;
        }
        sleep_ms(50);
    }
    tempo_estudo_config = opcoes_estudo[indice_estudo - 1];
    
    // Bloco 2: Configurar tempo de descanso curto
    uint indice_descanso = 1;  // 1 => 5 min, 2 => 10 min, 3 => 15 min
    const uint64_t opcoes_descanso[3] = { 5ULL * 60ULL * 1000000ULL, 10ULL * 60ULL * 1000000ULL, 15ULL * 60ULL * 1000000ULL };
    const char *texto_descanso[3] = { "5 min", "10 min", "15 min" };

    atualiza_exibicao_selecao("Descanso:", texto_descanso[indice_descanso - 1]);
    
    esperando_neutro = false;  // Reinicia o flag para este bloco
    while (true) {
        adc_select_input(0);  // Canal 1 para o eixo Y
        uint16_t valor_y = adc_read();
        // (Opcional) printf("Valor Y: %u\n", valor_y);
        
        if (valor_y > 1700 && valor_y < 2700) {
            esperando_neutro = false;
        }
        
        if (!esperando_neutro) {
            if (valor_y >= 2700) {
                indice_descanso++;
                if (indice_descanso > 3) indice_descanso = 1;
                atualiza_exibicao_selecao("Descanso:", texto_descanso[indice_descanso - 1]);
                esperando_neutro = true;
                sleep_ms(100);
            } else if (valor_y <= 1700) {
                if (indice_descanso > 1)
                    indice_descanso--;
                else
                    indice_descanso = 3;
                atualiza_exibicao_selecao("Descanso:", texto_descanso[indice_descanso - 1]);
                esperando_neutro = true;
                sleep_ms(100);
            }
        }
        
        if (gpio_get(BUTTON_JOY) == 0) {
            sleep_ms(300);
            break;
        }
        sleep_ms(50);
    }
    tempo_descanso_config = opcoes_descanso[indice_descanso - 1];

    // Exibe a configuração final
    ssd1306_fill(&ssd, false);
    sprintf(buf, "Estudo: %s", texto_estudo[indice_estudo - 1]);
    ssd1306_draw_string(&ssd, buf, 0, 0);
    sprintf(buf, "Descanso: %s", texto_descanso[indice_descanso - 1]);
    ssd1306_draw_string(&ssd, buf, 0, 10);
    ssd1306_draw_string(&ssd, "4 ciclos -> Reset", 0, 20);
    ssd1306_send_data(&ssd);
    sleep_ms(1000);
}

int main()
{
    PIO pio = pio0;
    bool ok;
    uint sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &pio_matrix_program);
    pio_matrix_program_init(pio, sm, offset, OUT_PIN);

    adc_init(); // Inicializa o periferico ADC
    adc_gpio_init(VRX_PIN); // Inicializa o pino do eixo x
    adc_gpio_init(VRY_PIN); // Inicializa o pino do eixo y

// Configuração do botão do joystick
    gpio_init(BUTTON_JOY);
    gpio_set_dir(BUTTON_JOY, GPIO_IN);
    gpio_pull_up(BUTTON_JOY);
    stdio_init_all(); // Inicializa a comunicacao serial

    i2c_init(I2C_PORT, 400 * 1000);

     gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
     gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
     gpio_pull_up(I2C_SDA); // Pull up the data line
     gpio_pull_up(I2C_SCL); // Pull up the clock line
     ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
     ssd1306_config(&ssd); // Configura o display
     ssd1306_send_data(&ssd); // Envia os dados para o display

     // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    setup_inicialicacao_softaware(pio, sm);
    // Aguarda o usuário pressionar o botão do joystick para iniciar a configuração
    while (gpio_get(BUTTON_JOY) != 0) {
        tight_loop_contents();
    }
    sleep_ms(300); // debounce

    // Chama o callback de configuração (que realiza os 2 blocos)
    configurar_tempos();

    // Após a configuração, as variáveis 'tempo_estudo_config' e 'tempo_descanso_config'
    // contêm os valores escolhidos (em microsegundos) e podem ser usados para os 4 ciclos do Pomodoro.
    // Por exemplo, você poderá iniciar os alarmes ou cronômetros com esses valores.

    while (true) {
        // Loop principal do projeto...
        tight_loop_contents();
    }
    
    return 0;
}
