#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "generated/ws2812.pio.h"

// ==========================================================
// INCLUSÕES OBRIGATÓRIAS PARA O WATCHDOG
// ==========================================================
#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"

// definições para I2C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C

// definições dos pinos
#define VRX_PIN 27          // eixo X (horizontal)
#define VRY_PIN 26          // eixo Y (vertical)
#define SW_PIN 22           // botão do joystick
#define LED_PIN_RED 13      // led vermelho
#define LED_PIN_BLUE 12     // led azul
#define LED_PIN_GREEN 11    // led verde
#define BUTTON_A 5          // inicia/pausa o jogo
#define BUTTON_B 6          // modo pontos
#define BUZZER_PIN 10       // buzzer
#define WS2812_PIN 7        // matriz WS2812 5x5
#define NUM_PIXELS 25       // 5x5 LEDs

// estados do jogo
enum { MENU_INICIAL, MODO_JOGO, MODO_PONTOS, GAME_OVER } estado = MENU_INICIAL;

// variáveis globais
bool jogo_pausado = false;
uint32_t last_time = 0;
uint32_t last_print_time = 0;
uint32_t button_a_press_time = 0;
uint32_t button_b_press_time = 0;
bool button_a_pressed = false;
bool button_b_pressed = false;
int nave_x = 60, nave_y = 28; // posição inicial da nave (centro)
int alvo_x = 20, alvo_y = 20; // posição inicial do alvo
int obstaculo_x = 100, obstaculo_y = 40; // posição inicial do obstáculo
int obstaculo_dy = 1; // direção vertical do obstáculo (1 = baixo, -1 = cima)
uint32_t pontuacao = 0;
uint32_t pontuacao_max = 0;
bool colisao_ativa = false; // controla estado de colisão
uint32_t game_over_start_time = 0; // tempo de início do Game Over
bool game_over_triggered = false; // controle do estado Game Over

// Variável para armazenar contagem de resets pelo Watchdog
uint32_t reset_count_wdt = 0;

// buffer para a matriz de LEDs
bool led_buffer[NUM_PIXELS] = {0};

// padrão apagado (todos os LEDs off)
const uint8_t padrao_apagado[25] = {0};

// função para definir um pixel WS2812
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// função para converter RGB em formato WS2812
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// função para definir padrões da matriz 5x5
void set_pattern(const char* pattern, uint8_t r, uint8_t g, uint8_t b) {
    // limpa o buffer
    for (int i = 0; i < NUM_PIXELS; i++) {
        led_buffer[i] = 0;
    }

    // mapeia o padrão 5x5 para os índices do led_buffer
    if (strcmp(pattern, "V") == 0) {
        // padrão V (verde, acerto de alvo)
        led_buffer[24] = 0; led_buffer[23] = 0; led_buffer[22] = 0; led_buffer[21] = 0; led_buffer[20] = 0; 
        led_buffer[15] = 1; led_buffer[16] = 0; led_buffer[17] = 0; led_buffer[18] = 0; led_buffer[19] = 1; 
        led_buffer[14] = 0; led_buffer[13] = 1; led_buffer[12] = 0; led_buffer[11] = 1; led_buffer[10] = 0; 
        led_buffer[5]  = 0; led_buffer[6]  = 0; led_buffer[7]  = 1; led_buffer[8]  = 0; led_buffer[9]  = 0; 
        led_buffer[4]  = 0; led_buffer[3]  = 0; led_buffer[2]  = 0; led_buffer[1]  = 0; led_buffer[0]  = 0; 
    } else if (strcmp(pattern, "X") == 0) {
        // padrão X (vermelho, obstáculo)
        led_buffer[24] = 1; led_buffer[23] = 0; led_buffer[22] = 0; led_buffer[21] = 0; led_buffer[20] = 1; 
        led_buffer[15] = 0; led_buffer[16] = 1; led_buffer[17] = 0; led_buffer[18] = 1; led_buffer[19] = 0; 
        led_buffer[14] = 0; led_buffer[13] = 0; led_buffer[12] = 1; led_buffer[11] = 0; led_buffer[10] = 0; 
        led_buffer[5]  = 0; led_buffer[6]  = 1; led_buffer[7]  = 0; led_buffer[8]  = 1; led_buffer[9]  = 0; 
        led_buffer[4]  = 1; led_buffer[3]  = 0; led_buffer[2]  = 0; led_buffer[1]  = 0; led_buffer[0]  = 1; 
    } else if (strcmp(pattern, "SETA") == 0) {
        // padrão seta (azul, modo pontos)
        led_buffer[24] = 1; led_buffer[23] = 0; led_buffer[22] = 0; led_buffer[21] = 0; led_buffer[20] = 1; 
        led_buffer[15] = 0; led_buffer[16] = 1; led_buffer[17] = 0; led_buffer[18] = 0; led_buffer[19] = 1; 
        led_buffer[14] = 0; led_buffer[13] = 0; led_buffer[12] = 1; led_buffer[11] = 0; led_buffer[10] = 1; 
        led_buffer[5]  = 0; led_buffer[6]  = 0; led_buffer[7]  = 0; led_buffer[8]  = 1; led_buffer[9]  = 1; 
        led_buffer[4]  = 1; led_buffer[3] = 1; led_buffer[2] = 1; led_buffer[1] = 1; led_buffer[0] = 1; 
    } else {
        // padrão apagado
        for (int i = 0; i < NUM_PIXELS; i++) {
            led_buffer[i] = 0;
        }
    }

    // atualiza a matriz com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++) {
        if (led_buffer[i]) {
            put_pixel(urgb_u32(r, g, b)); // liga o LED com a cor
        } else {
            put_pixel(0); // desliga o LED
        }
    }
}

// desenha a nave
void draw_nave(ssd1306_t *ssd, int x, int y) {
    ssd1306_rect(ssd, y, x, 8, 8, true, true); // base 8x8 preenchida
}

// desenha o obstáculo
void draw_obstaculo(ssd1306_t *ssd, int x, int y) {
    ssd1306_rect(ssd, y, x, 10, 10, true, true); // quadrado 10x10 preenchido
}

// desenha o alvo
void draw_alvo(ssd1306_t *ssd, int x, int y) {
    ssd1306_rect(ssd, y, x, 8, 8, true, false); // quadrado 8x8 não preenchido
}

// inicializa PWM
uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);
    return slice_num;
}

// interrupção
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - last_time > 200) { // debounce 200ms
        last_time = current_time;
        if (estado == MENU_INICIAL) {
            if (gpio == BUTTON_A) {
                estado = MODO_JOGO;
                jogo_pausado = false;
                pontuacao = 0;
                nave_x = 60;
                nave_y = 28;
                alvo_x = rand() % 113;
                alvo_y = rand() % 49;
                obstaculo_x = 120;
                obstaculo_y = rand() % 49;
                obstaculo_dy = (rand() % 2 == 0) ? 1 : -1; // direção vertical aleatória
                printf("Botão A pressionado - Inicia Jogo\n");
            } else if (gpio == BUTTON_B) {
                estado = MODO_PONTOS;
                printf("Botão B pressionado - Modo Pontos (Max: %lu)\n", pontuacao_max);
            }
        } else if (estado == MODO_JOGO && gpio == BUTTON_A) {
            if (colisao_ativa) {
                colisao_ativa = false;
                jogo_pausado = false;
            } else {
                jogo_pausado = !jogo_pausado;
            }
            printf("Botão A pressionado - %s\n", jogo_pausado ? "Pausado" : "Continuar");
        } else if (estado == MODO_PONTOS && gpio == BUTTON_B) {
            estado = MENU_INICIAL;
            printf("Botão B pressionado - Volta ao Menu\n");
        }
        
        // joystick aciona Game Over (somente se não estiver no menu para evitar conflito com teste de WDT)
        if (gpio == SW_PIN && estado != GAME_OVER && estado != MENU_INICIAL && !game_over_triggered) {
            estado = GAME_OVER;
            game_over_triggered = true;
            game_over_start_time = current_time;
            printf("Botão Joystick pressionado - Game Over\n");
        } else if (gpio == SW_PIN && estado == GAME_OVER && game_over_triggered) {
            estado = MENU_INICIAL;
            game_over_triggered = false;
            pontuacao = 0;
            printf("Botão Joystick pressionado - Volta ao Menu\n");
        }
    }
}

int main() {
    stdio_init_all();

    // ==========================================================
    // 1. VERIFICAÇÃO DE CAUSA DE RESET (Watchdog)
    // ==========================================================
    if (watchdog_caused_reboot()) {
        // Ocorreu reboot por WDT
        // Recupera o valor salvo no registrador scratch[0]
        reset_count_wdt = watchdog_hw->scratch[0];
        reset_count_wdt++; // Incrementa
        watchdog_hw->scratch[0] = reset_count_wdt; // Salva novamente
        printf("\n\n>>> Reiniciado pelo Watchdog! Contagem de resets: %d\n", reset_count_wdt);
    } else {
        // Reset normal (energia ou botão reset)
        printf(">>> Reset normal (Power On). Iniciando contador em 0.\n");
        watchdog_hw->scratch[0] = 0;
        reset_count_wdt = 0;
    }

    // ==========================================================
    // 2. HABILITA O WATCHDOG (4 segundos)
    // ==========================================================

    watchdog_enable(4000, 1); 
    printf("Watchdog habilitado (timeout = 4000 ms)\n");


    // configuração do ADC
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    // configuração dos botões
    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // configuração dos LEDs e buzzer
    uint pwm_wrap = 4096;
    uint pwm_slice_red = pwm_init_gpio(LED_PIN_RED, pwm_wrap);
    uint pwm_slice_blue = pwm_init_gpio(LED_PIN_BLUE, pwm_wrap);
    uint pwm_slice_green = pwm_init_gpio(LED_PIN_GREEN, pwm_wrap);
    uint pwm_slice_buzzer = pwm_init_gpio(BUZZER_PIN, 31250); // 4000Hz

    // configuração da matriz WS2812
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);

    // configuração I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // inicialização do display OLED
    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // configuração das interrupções
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(SW_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_FALL, true);

    while (true) {
        
        // ==========================================================
        // 3. ALIMENTAÇÃO DO WATCHDOG (Feed)
        // ==========================================================
        watchdog_update(); // Reseta o timer para 4000ms

        // leitura do joystick
        adc_select_input(0); // GPIO 26 (VRY)
        uint16_t vry_value = adc_read();
        adc_select_input(1); // GPIO 27 (VRX)
        uint16_t vrx_value = adc_read();

        // verificar pressão longa (Botão A e B) - Lógica original
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (!gpio_get(BUTTON_A)) {
            if (!button_a_pressed) {
                button_a_press_time = current_time;
                button_a_pressed = true;
            } else if (current_time - button_a_press_time >= 2500 && estado != MENU_INICIAL && estado != GAME_OVER) {
                estado = MENU_INICIAL;
                colisao_ativa = false;
                jogo_pausado = false;
                printf("Botão A pressionado por 2.5s - Volta ao Menu\n");
            }
        } else {
            button_a_pressed = false;
        }
        if (!gpio_get(BUTTON_B)) {
            if (!button_b_pressed) {
                button_b_press_time = current_time;
                button_b_pressed = true;
            } else if (current_time - button_b_press_time >= 2500 && estado != MENU_INICIAL && estado != GAME_OVER) {
                estado = MENU_INICIAL;
                colisao_ativa = false;
                jogo_pausado = false;
                printf("Botão B pressionado por 2.5s - Volta ao Menu\n");
            }
        } else {
            button_b_pressed = false;
        }

        // ==========================================================
        // 4. SIMULAÇÃO DE FALHA (TRAVAMENTO)
        // ==========================================================
        // Se estiver no MENU e segurar o Joystick (SW_PIN) pressionado
        if (estado == MENU_INICIAL && !gpio_get(SW_PIN)) {
            printf("\n!!! SIMULANDO TRAVAMENTO DE SOFTWARE !!!\n");
            printf("O Watchdog deve reiniciar o sistema em 4 segundos...\n");
            
            // Desenha aviso no display antes de travar
            // REMOVI "..." pois não existe na fonte
            ssd1306_fill(&ssd, false);
            ssd1306_draw_string(&ssd, "SIMULANDO", 20, 20);
            ssd1306_draw_string(&ssd, "TRAVAMENTO", 10, 30);
            ssd1306_send_data(&ssd);

            // Loop Infinito INTENCIONAL para estourar o Watchdog
            while(true) {
                // Pisca LED vermelho rápido indicando erro crítico
                pwm_set_gpio_level(LED_PIN_RED, 4096);
                sleep_ms(100);
                pwm_set_gpio_level(LED_PIN_RED, 0);
                sleep_ms(100);
                
                // O contador vai chegar a zero e resetar.
            }
        }

        // lógica por estado
        ssd1306_fill(&ssd, false);

        if (estado == MENU_INICIAL) {
            // desenha borda retangular nas extremidades
            ssd1306_rect(&ssd, 0, 0, 128, 128, true, false); 
            ssd1306_draw_string(&ssd, "    STARDOG", 4, 10);
            ssd1306_draw_string(&ssd, "    JOGAR A", 4, 25);
            ssd1306_draw_string(&ssd, "   PONTOS B", 4, 35);
            
            // Mostra status do Watchdog no menu
            char wdt_msg[20];
            if(reset_count_wdt > 0) {
                 snprintf(wdt_msg, sizeof(wdt_msg), "WDT RESET %d", reset_count_wdt);
            } else {
                 snprintf(wdt_msg, sizeof(wdt_msg), "BOOT NORMAL");
            }
            ssd1306_draw_string(&ssd, wdt_msg, 4, 50);

            pwm_set_gpio_level(LED_PIN_RED, 0);
            pwm_set_gpio_level(LED_PIN_GREEN, 0);
            pwm_set_gpio_level(LED_PIN_BLUE, 0);
            pwm_set_gpio_level(BUZZER_PIN, 0);
            set_pattern("APAGADO", 0, 0, 0); // matriz apagada
        } else if (estado == MODO_JOGO) {
            if (!jogo_pausado) {
                // movimento da nave
                nave_x = (vrx_value * 120) / 4095; // 0 a 120 pixels
                nave_y = (56 - (vry_value * 56) / 4095); // inverte Y: 0 (topo) a 56 (base)

                // garantir limites
                if (nave_x < 0) nave_x = 0;
                if (nave_x > 120) nave_x = 120;
                if (nave_y < 0) nave_y = 0;
                if (nave_y > 56) nave_y = 56;

                // movimento do obstáculo
                obstaculo_x--;
                obstaculo_y += obstaculo_dy;
                if (obstaculo_x < 0) {
                    obstaculo_x = 120;
                    obstaculo_y = rand() % 49;
                    obstaculo_dy = (rand() % 2 == 0) ? 1 : -1; // nova direção vertical
                }
                // limites verticais
                if (obstaculo_y <= 0) {
                    obstaculo_y = 0;
                    obstaculo_dy = 1; // inverte pra baixo
                } else if (obstaculo_y >= 56) {
                    obstaculo_y = 56;
                    obstaculo_dy = -1; // inverte pra cima
                }

                // verifica coleta de alvo
                if (abs(nave_x - alvo_x) < 8 && abs(nave_y - alvo_y) < 8) {
                    pontuacao++;
                    if (pontuacao > pontuacao_max) pontuacao_max = pontuacao;
                    alvo_x = rand() % 113;
                    alvo_y = rand() % 49;
                    // desligar todos os LEDs antes
                    pwm_set_gpio_level(LED_PIN_RED, 0);
                    pwm_set_gpio_level(LED_PIN_GREEN, 0);
                    pwm_set_gpio_level(LED_PIN_BLUE, 0);
                    // acender LED verde e padrão V
                    pwm_set_gpio_level(LED_PIN_GREEN, 4096);
                    pwm_set_gpio_level(BUZZER_PIN, 3500); // ~300Hz
                    set_pattern("V", 0, 1, 0); // V verde
                    
                    sleep_ms(300);
                    
                    pwm_set_gpio_level(LED_PIN_GREEN, 0);
                    pwm_set_gpio_level(BUZZER_PIN, 0);
                    set_pattern("APAGADO", 0, 0, 0); // matriz apagada
                    printf("Alvo coletado: PTS=%lu, Max=%lu\n"); 
                }

                // verifica colisão com obstáculo
                if (abs(nave_x - obstaculo_x) < 10 && abs(nave_y - obstaculo_y) < 10 && !colisao_ativa) {
                    colisao_ativa = true;
                    jogo_pausado = true;
                    // desligar todos os LEDs antes
                    pwm_set_gpio_level(LED_PIN_RED, 0);
                    pwm_set_gpio_level(LED_PIN_GREEN, 0);
                    pwm_set_gpio_level(LED_PIN_BLUE, 0);
                    // acender LED vermelho e padrão X
                    pwm_set_gpio_level(LED_PIN_RED, 4096);
                    pwm_set_gpio_level(BUZZER_PIN, 10000); // ~2000Hz
                    set_pattern("X", 1, 0, 0); // X vermelho
                    sleep_ms(550);
                    pwm_set_gpio_level(LED_PIN_RED, 0);
                    pwm_set_gpio_level(BUZZER_PIN, 0);
                    set_pattern("APAGADO", 0, 0, 0); // matriz apagada
                    printf("Colisão com obstáculo\n");  
                }
            }

            // desenha no display
            draw_nave(&ssd, nave_x, nave_y); // nave
            draw_alvo(&ssd, alvo_x, alvo_y); // alvo
            draw_obstaculo(&ssd, obstaculo_x, obstaculo_y); // obstáculo
            char pts_str[16];
            snprintf(pts_str, sizeof(pts_str), "PTS %lu", pontuacao);
            ssd1306_draw_string(&ssd, pts_str, 0, 0);
            if (jogo_pausado) {
                ssd1306_draw_string(&ssd, "PAUSADO", 40, 28);
            }
        } else if (estado == MODO_PONTOS) {
            char max_pts_str[16];
            snprintf(max_pts_str, sizeof(max_pts_str), "MAX %lu", pontuacao_max);
            ssd1306_draw_string(&ssd, "PONTUACAO MAX", 0, 20);
            ssd1306_draw_string(&ssd, max_pts_str, 40, 32);
            pwm_set_gpio_level(LED_PIN_RED, 0);
            pwm_set_gpio_level(LED_PIN_GREEN, 0);
            pwm_set_gpio_level(LED_PIN_BLUE, 4096); // LED azul
            pwm_set_gpio_level(BUZZER_PIN, 0);
            set_pattern("SETA", 0, 0, 1); // seta azul
        } else if (estado == GAME_OVER) {
            // exibe "GAME OVER" por 2 segundos
            if (current_time - game_over_start_time < 2000) {
                ssd1306_draw_string(&ssd, "GAME OVER", 40, 28);
                pwm_set_gpio_level(LED_PIN_RED, 0);
                pwm_set_gpio_level(LED_PIN_GREEN, 0);
                pwm_set_gpio_level(LED_PIN_BLUE, 0);
                pwm_set_gpio_level(BUZZER_PIN, 0);
                set_pattern("APAGADO", 0, 0, 0);
            } else {
                // tudo apagado após 2 segundos
                ssd1306_fill(&ssd, false); // display apagado
                pwm_set_gpio_level(LED_PIN_RED, 0);
                pwm_set_gpio_level(LED_PIN_GREEN, 0);
                pwm_set_gpio_level(LED_PIN_BLUE, 0);
                pwm_set_gpio_level(BUZZER_PIN, 0);
                set_pattern("APAGADO", 0, 0, 0);
            }
        }

        ssd1306_send_data(&ssd);

        // UART a cada 1s
        if (current_time - last_print_time >= 1000) {
            // info do watchdog no print de debug
            printf("Nave: (%d, %d), PTS: %lu, WDT Resets: %d\n", nave_x, nave_y, pontuacao, reset_count_wdt); 
            last_print_time = current_time;
        }

        sleep_ms(50);
    }

    return 0;
}