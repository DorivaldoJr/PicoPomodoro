#include "buzzer.h"

static uint buzzer_pin;
static uint buzzer_slice;
static uint buzzer_channel;

// Callback para desligar o buzzer (chamado via alarme)
static int64_t buzzer_off_callback(alarm_id_t id, void *user_data) {
    pwm_set_chan_level(buzzer_slice, buzzer_channel, 0);
    return -1; // Não reagenda
}

// Função auxiliar para tocar um tom pelo tempo indicado (em ms)
static void buzzer_play_tone(uint frequency, uint duration_ms) {
    const uint32_t clock_freq = 125000000; // Exemplo: 125MHz (clock do RP2040)
    uint32_t wrap = clock_freq / frequency;
    // Configure o wrap e o duty cycle (50% de duty cycle)
    pwm_set_wrap(buzzer_slice, wrap);
    pwm_set_chan_level(buzzer_slice, buzzer_channel, wrap / 2);
    // Agende o desligamento após duration_ms
    add_alarm_in_ms(duration_ms, buzzer_off_callback, NULL, true);
}

void buzzer_init(uint gpio_pin) {
    buzzer_pin = gpio_pin;
    gpio_set_function(buzzer_pin, GPIO_FUNC_PWM);
    buzzer_slice = pwm_gpio_to_slice_num(buzzer_pin);
    buzzer_channel = pwm_gpio_to_channel(buzzer_pin);
    // Inicia com o canal desligado
    pwm_set_chan_level(buzzer_slice, buzzer_channel, 0);
    pwm_set_enabled(buzzer_slice, true);
}

void buzzer_inicio(void) {
    // Toca 2000Hz por 100ms
    buzzer_play_tone(2000, 100);
}

void buzzer_vitoria(void) {
    // Toca 1500Hz por 200ms
    buzzer_play_tone(1500, 200);
}

void buzzer_musiquinha(void) {
    // Toca 1000Hz por 500ms
    buzzer_play_tone(1000, 500);
}