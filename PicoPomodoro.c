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
#include "hardware/watchdog.h"

static alarm_id_t alarm_leds = 0;  // ID do alarme para os LEDs

  
static bool timer_ativo = false;  // Flag para indicar se o timer está rodando

// Macros I2c
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// Macros Matriz led's
#define NUM_PIXELS 25 //matriz led
#define OUT_PIN 7 // matriz led

PIO pio_global;         
uint matrix_sm_global;  


// Macros botoes e led RGB
#define BUTTON_JOY 22
#define BUTTON_A 5
#define BUTTON_B 6
#define LED_RED 13
#define LED_GREEN 11

#define VRY_PIN 27 // Pino para o ADC no eixo y

#define TIME_25_MIN_US_ESTUDO 1500000000ULL // 25 min convertidos para microsegundos
#define TIME_30_MIN_US_ESTUDO 1800000000ULL // 30 min convertidos para microsegundos
#define TIME_35_MIN_US_ESTUDO 2100000000ULL // 35 min convertidos para microsegundos
#define TIME_PAUSA_CURTA_5_MIN_US   (5ULL * 60ULL * 1000000ULL)   // 5 minutos para micro segundos
#define TIME_PAUSA_CURTA_10_MIN_US   (10ULL * 60ULL * 1000000ULL)  // 10 min para micro segundos
#define TIME_PAUSA_CURTA_15_MIN_US   (15ULL * 60ULL * 1000000ULL)  // 15 min para micro segundos


ssd1306_t ssd; // Inicializa a estrutura do display, esta aqui para facilitar a chamada e reduzir erros de compilacao

static volatile uint estudo = 0; // Variavel de controle para contar os timers de estudo
static volatile uint repouso = 0; // Variavel de controle para contar os timers de repouso
static alarm_id_t alarm_estudo = 0;



void setup_led_buttons (){
    // Configuração do botão do joystick
    gpio_init(BUTTON_JOY);
    gpio_set_dir(BUTTON_JOY, GPIO_IN);
    gpio_pull_up(BUTTON_JOY);

    // Configuracao dos botoes A e B
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // Configuracao do Led
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
}

void setup_inicialicacao_softaware (PIO pio, uint sm){ // Inicializa o softaware com as boas vindas e instrucoes para o usuario 
    // Inicializcao serial
    printf("Bem vindo ao PicoPomodoro!\n"); 
    // Inicializacao via Display
    ssd1306_rect(&ssd,0,0,128,64,true,false);
    ssd1306_draw_string(&ssd, "BEM VINDO AO", 15, 3);
    ssd1306_draw_string(&ssd, "PICO POMODORO!", 12, 15);
    ssd1306_draw_string(&ssd, "Pressione o", 3, 32);
    ssd1306_draw_string(&ssd, "joystick para", 3 , 42);
    ssd1306_draw_string(&ssd, "configurar", 3, 52);
    ssd1306_send_data(&ssd);
   // Inicializacao pela matriz de leds
    animacao_inicio(pio, sm);
}


uint64_t tempo_estudo_config = 0;   // Valor final em microsegundos para ciclo de estudo
uint64_t tempo_descanso_config = 0; // Valor final em microsegundos para pausa curta


// Funcao para configurar o tempo dos ciclos de estudo e o tempo de cada pausa pelo usuario
// Utiliza o joystick no eixo y para selecionar o tempo de cada ciclo 
void configurar_tempos(void) {

    char buf[32];  // Buffer para sprintf

    // -------------------------------------------
    // ** Bloco 1: Configura tempo de estudo **
    // --------------------------------------------
    uint indice_estudo = 1;  // 1 => 25 min, 2 => 30 min, 3 => 35 min. Indice do vetor com os tempos
    const uint64_t opcoes_estudo[3] = { TIME_25_MIN_US_ESTUDO, TIME_30_MIN_US_ESTUDO, TIME_35_MIN_US_ESTUDO }; // Vetor com os tempos convertidos
    const char *texto_estudo[3] = { "25 min", "30 min", "35 min" }; // Vetor de texto para atualizar o display com o tempo escolhido
    ssd1306_fill(&ssd, false);
    ssd1306_rect(&ssd,0,0,128,64,true,false);
    ssd1306_draw_string(&ssd, "Tempo de", 3,3);
    ssd1306_draw_string(&ssd, "1 Joy no eixo Y", 3,42);
    ssd1306_draw_string(&ssd, "2 Botao do Joy", 3,52);
    atualiza_exibicao_selecao(&ssd,"Estudo:", texto_estudo[indice_estudo - 1]); // Mostra o ciclo com tempo defaut 25 min
    
    bool esperando_neutro = false; // variavel para indicar zona neutra do joystick

    while (true) {
        adc_select_input(0);  // Canal 0 para o eixo Y (GP27)
        uint16_t valor_y = adc_read(); // Le o valor do canal e manda para a variavel 
        
        // Define faixa neutra: entre 1700 e 2700
        if (valor_y > 1700 && valor_y < 2700) {
            esperando_neutro = false;
        }
        
        if (!esperando_neutro) {
            if (valor_y >= 2700) {
                indice_estudo++; // Incrementa o indice do vetor com os tempos se o joystick for levantado
                if (indice_estudo > 3) indice_estudo = 1; // Reseta o indice se chegar ao limite do vetor
                atualiza_exibicao_selecao(&ssd, "Estudo:", texto_estudo[indice_estudo - 1]); // Atualizacao da mudanca no display
                esperando_neutro = true; // Atualiza a variavel para voltar ao loop
                sleep_ms(100); // Tempo para nao sobrecarregar o sistema 
                } else if (valor_y <= 1700) {
                if (indice_estudo > 1)
                    indice_estudo--; // Decrementa o indice se o joystick for abaixado
                else
                    indice_estudo = 3; // Reseta se caso chegar no limite abaixo
                atualiza_exibicao_selecao(&ssd, "Estudo:", texto_estudo[indice_estudo - 1]); // Atualizacao do display em tempo real
                esperando_neutro = true; // Reseta a variavel para voltar ao loop
                sleep_ms(100); // Tempo para nao sobrecarregar o sistema 
            }
        }
        
        // Se o usuário confirmar pressionando o botão do joystick:
        if (gpio_get(BUTTON_JOY) == 0) {
            sleep_ms(300); // debounce do botão
            break;  // Sai do loop e vai para o bloco 2 de configuracao
        }
        sleep_ms(50); // Pausa do sistema
    }
    tempo_estudo_config = opcoes_estudo[indice_estudo - 1]; // Armazena o ultimo tempo escolhido pelo usuario depois de clicar no botao
    // Posteriormente o tempo sera usado para configurar o alarm
    
    // -------------------------------------------------
    // ** Bloco 2: Configurar tempo de descanso curto **
    // -------------------------------------------------
    uint indice_descanso = 1;  // 1 => 5 min, 2 => 10 min, 3 => 15 min. Indice do vetor com os tempos
    const uint64_t opcoes_descanso[3] = { 5ULL * 60ULL * 1000000ULL, 10ULL * 60ULL * 1000000ULL, 15ULL * 60ULL * 1000000ULL }; // Vetor com os tempos convertidos
    const char *texto_descanso[3] = { "05 min", "10 min", "15 min" }; // Vetor com os tempos escritos para o display

    atualiza_exibicao_selecao(&ssd, "Pausa:", texto_descanso[indice_descanso - 1]); // Atualizacao com tempo defaut de 5 min
    
    esperando_neutro = false;  // Reinicia o flag para este bloco
    while (true) {
       
        adc_select_input(0);  // Canal 0 para o eixo Y
        uint16_t valor_y = adc_read(); // Le o valor do canal e manda para a variavel 
        
        // Define faixa neutra: entre 1700 e 2700
        if (valor_y > 1700 && valor_y < 2700) {
            esperando_neutro = false;
        }
        
        if (!esperando_neutro) {
            if (valor_y >= 2700) {
                indice_descanso++; // Incrementa o indice do vetor com os tempos se o joystick for levantado
                if (indice_descanso > 3) indice_descanso = 1; // Reseta o indice se chegar ao limite do vetor
                atualiza_exibicao_selecao(&ssd, "Pausa:", texto_descanso[indice_descanso - 1]); // Atualizacao da mudanca no display
                esperando_neutro = true; // Atualiza a variavel para voltar ao loop
                sleep_ms(100);
            } else if (valor_y <= 1700) {
                if (indice_descanso > 1)
                    indice_descanso--; // Decrementa o indice se o joystick for abaixado
                else
                    indice_descanso = 3; // Reseta se caso chegar no limite abaixo
                atualiza_exibicao_selecao(&ssd, "Pausa:", texto_descanso[indice_descanso - 1]); // Atualizacao da mudanca no display
                esperando_neutro = true; // Atualiza a variavel para voltar ao loop
                sleep_ms(100);
            }
        }
        
        // Se o usuário confirmar pressionando o botão do joystick:
        if (gpio_get(BUTTON_JOY) == 0) {
            sleep_ms(300); // debounce do botão
            break; // Sai do loop e vai para o bloco 2 de configuracao
        }
        sleep_ms(50); // Pausa para o sistema
    }
    tempo_descanso_config = opcoes_descanso[indice_descanso - 1]; // Armazena o ultimo tempo escolhido pelo usuario depois de clicar no botao
    // Posteriormente o tempo sera usado para configurar o alarm

    // Exibe a configuração final
    ssd1306_fill(&ssd, false);
    ssd1306_rect(&ssd,0,0,128,64,true,false);
    sprintf(buf, "Estudo: %s", texto_estudo[indice_estudo - 1]);
    ssd1306_draw_string(&ssd, buf, 3, 3);
    sprintf(buf, "Pausa: %s", texto_descanso[indice_descanso - 1]);
    ssd1306_draw_string(&ssd, buf, 3, 13);
    ssd1306_draw_string(&ssd, "4 ciclos estudo", 3, 23);
    ssd1306_draw_string(&ssd, "Start botao A", 3, 43);
    ssd1306_send_data(&ssd);
    
} // fim do lop de configuracao 

int64_t led_callback(alarm_id_t id, void *user_data) {
    if (led_steps < led_max_steps) {
        acender_led_sequencialmente();
        led_steps++;
        return (tempo_estudo_config / led_max_steps);
    }

    // **Apaga os LEDs ao final do estudo**
    for (int i = 0; i < NUM_PIXELS; i++) {
        led_buffer[i] = matrix_rgb(0.0, 0.0, 0.0);
    }
    update_led_matrix();

    led_steps = 0;
    alarm_leds = 0;

    return 0;
}



int64_t timers_callback(alarm_id_t id, void *user_data) {
    if ((estudo ) == 4 && repouso == 3) {
        
        // **Aqui garantimos que os LEDs apaguem no início da pausa**
        for (int i = 0; i < NUM_PIXELS; i++) {
            led_buffer[i] = matrix_rgb(0.0, 0.0, 0.0);  // Apaga todos os LEDs
        }
        ssd1306_fill(&ssd, false);
        ssd1306_rect(&ssd,0,0,128,64,true,false);
        ssd1306_draw_string(&ssd, "PARABENS!",3, 8);
        
        ssd1306_draw_string(&ssd, "Fim do Pomodoro",3, 18);
        ssd1306_draw_string(&ssd, "Aperte B para",3, 38);
        ssd1306_draw_string(&ssd, "recomecar",3, 48);
        ssd1306_send_data(&ssd);
        timer_ativo = false;
        animacao_inicio(pio_global,matrix_sm_global);
       
        return -1;
    }
    
    if (estudo > repouso) {
        
        ssd1306_fill(&ssd, false);
        ssd1306_rect(&ssd,0,0,128,64,true,false);
        ssd1306_draw_string(&ssd, "Timer de pausa",9, 18);
        ssd1306_draw_string(&ssd, "Iniciado!",32, 28);
        ssd1306_draw_string(&ssd, "Bom descanso!",14, 38);
        ssd1306_send_data(&ssd);
        repouso++;
        // **Aqui garantimos que os LEDs apaguem no início da pausa**
        for (int i = 0; i < NUM_PIXELS; i++) {
            led_buffer[i] = matrix_rgb(0.0, 0.0, 0.0);  // Apaga todos os LEDs
        }
        update_led_matrix();  // Atualiza a matriz para refletir o desligamento
        return tempo_descanso_config;
    }

    if (estudo == repouso) {
        estudo++;
        switch (estudo){
         case 2 :
        ssd1306_fill(&ssd, false);
        ssd1306_rect(&ssd,0,0,128,64,true,false);
        ssd1306_draw_string(&ssd, "Segundo timer",9, 18);
        ssd1306_draw_string(&ssd, "Iniciado!",32, 28);
        ssd1306_draw_string(&ssd, "Bons estudos!",14, 38);
        ssd1306_send_data(&ssd);
        
        break;
        case 3 :
        ssd1306_fill(&ssd, false);
        ssd1306_rect(&ssd,0,0,128,64,true,false);
        ssd1306_draw_string(&ssd, "Terceiro timer",9, 18);
        ssd1306_draw_string(&ssd, "Iniciado!",32, 28);
        ssd1306_draw_string(&ssd, "Bons estudos!",14, 38);
        ssd1306_send_data(&ssd);
        
        break;
        case 4 :
        ssd1306_fill(&ssd, false);
        ssd1306_rect(&ssd,0,0,128,64,true,false);
        ssd1306_draw_string(&ssd, "Ultimo timer",13, 18);
        ssd1306_draw_string(&ssd, "Iniciado!",32, 28);
        ssd1306_draw_string(&ssd, "Bons estudos!",14, 38);
        ssd1306_send_data(&ssd);
        
        }
        // **Inicia o cronômetro dos LEDs SOMENTE no estudo**
        if (alarm_leds == 0) {
            led_steps = 0;
            alarm_leds = add_alarm_in_us(tempo_estudo_config / led_max_steps, led_callback, NULL, false);
        }
        
        return tempo_estudo_config;
    }

    return 0;
}




void callback_button(uint gpio, uint32_t events) {
    static uint32_t last_time_A = 0;
    static uint32_t last_time_B = 0;
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if(gpio == BUTTON_A){
        if (current_time - last_time_A > 200000) {
            last_time_A = current_time;
            
            ssd1306_fill(&ssd, false);
            ssd1306_rect(&ssd,0,0,128,64,true,false);
            ssd1306_draw_string(&ssd, "Primeiro timer",9, 18);
            ssd1306_draw_string(&ssd, "Iniciado",32, 28);
            ssd1306_draw_string(&ssd, "Bons estudos",16, 38);
            ssd1306_send_data(&ssd);
            estudo = 1;
            repouso = 0;
            timer_ativo = true;  // Agora o timer está ativo

            alarm_estudo = add_alarm_in_us(tempo_estudo_config, timers_callback, NULL, true);
            
            // **Ativa LEDs somente se não estiverem rodando**
            if (alarm_leds == 0) {
                led_steps = 0;
                alarm_leds = add_alarm_in_us(tempo_estudo_config / led_max_steps, led_callback, NULL, false);
            }
            
        }
    } 
     if (gpio == BUTTON_B){

        if (current_time - last_time_B > 200000) { // Debounce de 200 ms
        last_time_B = current_time;
    
        watchdog_reboot(0, 0, 0);
        
        
        }

    }
}




int main()
{
    // Inicializa o PIO
    PIO pio = pio0;
    bool ok;
    uint sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &pio_matrix_program);
    pio_matrix_program_init(pio, sm, offset, OUT_PIN);
    // Atribui os valores globais para que o callback os acesse
    pio_global = pio;
    matrix_sm_global = sm;

    // Inicializa o ADC
    adc_init(); // Inicializa o periferico ADC
    adc_gpio_init(VRY_PIN); // Inicializa o pino do eixo y

    stdio_init_all(); // Inicializa a comunicacao serial

    setup_led_buttons(); // Inicializa os leds e os botoes

    // Inicializa o I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    
    // Inicializacao do display ssd1306
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    setup_inicialicacao_softaware(pio, sm); // Inicializa a interface de boas vindas do programa

    // Aguarda o usuário pressionar o botão do joystick para iniciar a configuração
    while (gpio_get(BUTTON_JOY) != 0) {
        tight_loop_contents();
    }
    sleep_ms(300); // debounce

    // Chama o callback de configuração (que realiza os 2 blocos)
    configurar_tempos();

    // Interrupcao do botao A para iniciar o timer
    gpio_set_irq_enabled_with_callback(BUTTON_A,GPIO_IRQ_EDGE_FALL, true, &callback_button);
     gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &callback_button);
  
    while (true) {
    if (timer_ativo) {  
        if (estudo > repouso) {
            // **Não inicializa LEDs no descanso**
        } else if (estudo == repouso) {
            if (alarm_leds == 0) {
                led_steps = 0;
                alarm_leds = add_alarm_in_us(tempo_estudo_config / led_max_steps, led_callback, NULL, false);
            }
        }
    }
    tight_loop_contents();
    }
    
    return 0;
}
