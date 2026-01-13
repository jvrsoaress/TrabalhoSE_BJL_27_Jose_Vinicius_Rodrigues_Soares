<img width=100% src="https://capsule-render.vercel.app/api?type=waving&color=02A6F4&height=120&section=header"/>
<h1 align="center">Embarcatech - Projeto Integrado - BitDogLab </h1>

## 🎯 Objetivos do Projeto

O objetivo principal é demonstrar o uso do Watchdog para garantir que o sistema se recupere automaticamente de falhas de software (travamentos). O projeto implementa:
1.  **Configuração do Watchdog** com timeout adequado.
2.  **Alimentação (Feed)** periódica do timer durante a operação normal.
3.  **Simulação de Falha** intencional para testar o mecanismo.
4.  **Diagnóstico de Reset** para identificar se o reinício foi causado pelo WDT.

---

## 🐶 Implementação do Watchdog Timer

O sistema foi programado para operar com um **timeout de 4 segundos** (4000ms). Durante o funcionamento normal do jogo, o Watchdog é alimentado (`watchdog_update`) dentro do loop principal.

### ✅ Justificativa dos Requisitos

Abaixo, detalhamos como cada requisito da tarefa foi atendido no código:

| Requisito | Implementação no Projeto |
| :--- | :--- |
| **Configurar e Operar** | O WDT é ativado com `watchdog_enable(4000, 1)`, definindo um tempo limite de 4 segundos, suficiente para cobrir a latência das tarefas do jogo sem disparos falsos. |
| **Gerenciar o "Feed"** | A função `watchdog_update()` é chamada a cada iteração do loop principal (`while(true)`), garantindo que o sistema não reinicie enquanto o jogo estiver fluindo corretamente. |
| **Validar a Resiliência** | Foi criado um mecanismo de teste no Menu Inicial. Ao segurar o **Botão do Joystick**, o código entra propositalmente em um loop infinito *sem* alimentar o WDT, forçando o estouro do temporizador. |
| **Diagnosticar Resets** | Ao iniciar, o sistema verifica `watchdog_caused_reboot()`. Se verdadeiro, incrementa um contador salvo no registrador `scratch[0]` e exibe a mensagem **"WDT RESET [N]"** no display OLED, informando ao usuário que houve uma recuperação de falha. |

---

## 🧪 Como Testar o Watchdog (Guia Passo a Passo)

Para verificar o funcionamento da proteção contra travamentos, siga este roteiro:

1.  **Boot Normal:**
    * Ligue a placa BitDogLab ou reinicie-a.
    * No **Menu Inicial** (tela com título STARDOG), observe no rodapé a mensagem: **"BOOT NORMAL"**.

2.  **Simular o Travamento:**
    * Ainda no Menu Inicial, pressione e segure o **Botão do Joystick (SW/Pino 22)**.
    * O sistema simulará um erro crítico:
        * O display exibirá: **"SIMULANDO TRAVAMENTO"**.
        * O **LED Vermelho** piscará rapidamente.
        * O sistema parará de responder (loop infinito).
    * *Nota: Neste momento, o código para de alimentar o Watchdog propositalmente.*

3.  **Observar a Recuperação:**
    * Aguarde aproximadamente **4 segundos**.
    * O Watchdog detectará a falta de resposta e reiniciará o microcontrolador automaticamente.

4.  **Verificar o Diagnóstico:**
    * Assim que o sistema reiniciar, olhe novamente para o Menu Inicial.
    * A mensagem no rodapé terá mudado para: **"WDT RESET 1"** (ou o número de vezes que você testou).
    * Isso confirma que o sistema identificou a falha e se recuperou com sucesso.

---

## 🎮 Sobre o Projeto Base (StarDog)

O "StarDog" é um jogo interativo desenvolvido para a placa BitDogLab. O jogador controla uma nave (quadrado 8x8) usando o joystick, com o objetivo de coletar alvos e evitar obstáculos.

### Tecnologias e Periféricos Utilizados
* **Microcontrolador:** Raspberry Pi Pico W (RP2040).
* **Display OLED SSD1306 (I2C):** Exibe o jogo e status do Watchdog.
* **Joystick (ADC):** Controla a nave e aciona a simulação de falha (botão SW).
* **LED RGB:** Feedback visual (Verde = Alvo, Vermelho = Colisão/Erro WDT).
* **Matriz de LEDs WS2812:** Exibe ícones de status (V, X, Seta).
* **Buzzer (PWM):** Feedback sonoro.
* **Botões A e B (IRQ):** Controle de fluxo do jogo.

### Funcionalidades do Jogo
* **Menu Inicial:** Seleção de modos e exibição do status de boot (Normal ou WDT).
* **Modo Jogo:** Controle da nave, pontuação e detecção de colisão.
* **Modo Pontos:** Exibição da pontuação máxima (High Score).
* **Game Over:** Tela de fim de jogo.

---

## 🚀 Compilação e Upload

1.  **Clonar o repositório:**
    ```sh
    git clone <link-do-seu-repositorio>
    ```

2.  **Configurar e compilar:**
    Certifique-se de ter o Pico SDK configurado.
    ```sh
    mkdir build && cd build
    cmake ..
    make
    ```

3.  **Transferir o firmware:**
    * Conecte a placa BitDogLab ao computador segurando o botão BOOTSEL.
    * Copie o arquivo `.uf2` gerado na pasta `build` para o drive `RPI-RP2`.

---

## 💻 Desenvolvedor

**José Vinicius**
