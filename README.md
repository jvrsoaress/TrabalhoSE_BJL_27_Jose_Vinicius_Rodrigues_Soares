<img width=100% src="https://capsule-render.vercel.app/api?type=waving&color=02A6F4&height=120&section=header"/>
<h1 align="center">Embarcatech - Projeto Integrado - BitDogLab </h1>

## Objetivo do Projeto

Desenvolver um jogo interativo no microcontrolador RP2040 usando a placa BitDogLab, onde o jogador controla uma nave, que trata-se de um quadrado 8x8, e a partir do uso do joystick é possível coletar alvos (quadrado 8x8) e evitar obstáculos (quadrado 10x10). Ademais, tem-se o feedback visual a partir do display OLED, os LEDs RGB, a matriz de LED WS2812 e o buzzer, além da exibição da pontuação do jogador dentro do game e também os estados do jogo, como o menu inicial, a tela do jogo, os pontos, e o Game Over.

## 🗒️ Lista de requisitos

- **Leitura analógica por meio do potenciômetro do joystick**: utilizando o conversor ADC do
RP2040;
- **Leitura de botões físicos (push-buttons)**: com tratamento de debounce, essencial para garantir a
confiabilidade das entradas digitais;
- **Utilização da matriz de LEDs, do LED RGB e do buzzer**: como saídas para feedback visual e
sonoro;
- **Exibição de informações em tempo real no display gráfico 128x64 (SSD1306)**: que se comunica
com o RP2040 via interface I2C;
- **Transmissão de dados e mensagens de depuração através da interface UART**: permitindo a
visualização em um terminal serial no computador;
- **Uso de interrupções**: Todas as funcionalidades relacionadas aos botões devem ser implementadas utilizando rotinas de interrupção (IRQ); 
- **Debouncing**: É obrigatório implementar o tratamento do bouncing dos botões via software; 
- **Estruturação do projeto no ambiente VS Code**: previamente configurado para o desenvolvimento
com o RP2040.

## 🛠 Tecnologias

1. **Microcontrolador**: Raspberry Pi Pico W (na BitDogLab).
2. **Display OLED SSD1306**: 128x64 pixels, conectado via I2C (GPIO 14 - SDA, GPIO 15 - SCL).
3. **LEDs RGB**:
- Verde: GPIO 11 (PWM).
- Azul: GPIO 12 (PWM).
- Vermelho: GPIO 13 (PWM).
4. **Joystick**:
- Eixo Y (VRY): GPIO 26 (ADC).
- Eixo X (VRX): GPIO 27 (ADC, usado no modo Dados).
- Botão (SW): GPIO 22 (interrupção).
5. **Botões**:
- Botão A: GPIO 5 (interrupção).
- Botão B: GPIO 6 (interrupção).
6. **Buzzer**: GPIO 10 (PWM).
7. **Matriz de LEDs: WS2812** (GPIO 7).
7. **Linguagem de Programação:** C  
8. **Frameworks:** Pico SDK


## 🔧 Funcionalidades Implementadas:

**Funções dos Componentes**
   
- Display: Exibe o menu inicial e as telas de cada modo (JOGAR e PONTOS).
- LEDs: Indicam estados (verde = ALVO, vermelho = OBSTÁCULO).
- Matriz de LEDs: Exibe "V" (verde, alvo), "X" (vermelho, obstáculo), e "SETA" (azul, modo pontos).
- Joystick: Controla a NAVE.
- Botões: Navegam entre modos e telas.
- Buzzer: Emite sons a depender de onde a NAVE atingir.

## 🔧 Fluxograma Geral:

- **Menu Inicial:** Exibe uma borda retangular ao redor do display, além do título "STARDOG" do jogo, e as opções "JOGAR: A" e "PONTOS: B". O jogador pode usar os botões A para iniciar o jogo ou B para ver pontuação máxima.
- **Modo Jogo:** A nave (um quadrado 8x8 preenchido) é controlada pelo joystick, movendo-se proporcionalmente no display. O jogador coleta alvos (um quadrado 8x8 não preenchido/”em branco”), ganhando pontos, e evita obstáculos (um quadrado 10x10 preenchido) que se movem da direita pra esquerda com deslocamento vertical aleatório. Ademais, a colisão da nave com o alvo acende o LED verde e exibe "V" na matriz de LED, além de tocar um som no buzzer. De maneira análoga, a colisão da nave com o obstáculo acende o LED vermelho, exibindo "X" na matriz de LED, e toca um som no buzzer pausando o jogo. Por fim, o botão A pausa/continua.
- **Modo Pontos:** Exibe a pontuação máxima que o jogador conseguir, acende LED azul, e mostra padrão uma "SETA" inclinada na matriz de LED. Caso o Botão B seja apertado, volta ao menu.
- **Game Over:** Caso o joystick seja apertado, aciona o Game Over, que faz com que seja exibido uma mensagem de "GAME OVER" no display por 2 segundos, desligando o sistema. Por fim, caso seja apertado mais uma vez o joystick, o sistema religa novamente voltando ao menu inicial.
- Pressão longa por 2.5s nos botões A ou B retorna ao menu, e as mensagens de depuração são enviadas via UART.

## 🚀 Passos para Compilação e Upload  

1. **Clonar o repositório** 

- sh
- git clone seu repositorio


2. **Configurar e compilar o projeto**  

`mkdir build && cd build`
`cmake ..`
`make`

3. **Transferir o firmware para a placa**

- Conectar a placa BitDogLab ao computador
- Copiar o arquivo .uf2 gerado para o drive da placa

4. **Testar o projeto**

🛠🔧🛠🔧🛠🔧


## 🎥 Demonstração: 

- Para ver o funcionamento do projeto, acesse o vídeo de demonstração gravado por José Vinicius em: https://youtu.be/UlLR2-UypfE

## 💻 Desenvolvedor
 
<table>
  <tr>
    <td align="center"><img style="" src="https://avatars.githubusercontent.com/u/191687774?v=4" width="100px;" alt=""/><br /><sub><b> José Vinicius </b></sub></a><br />👨‍💻</a></td>
  </tr>
</table>
