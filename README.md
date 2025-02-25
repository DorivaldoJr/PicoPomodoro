PicoPomodoro: Sistema de Gerenciamento de Tempo
===============================================

Introdução:
-----------
O PicoPomodoro é um sistema compacto de gerenciamento de tempo, desenvolvido para o Raspberry Pi Pico/Pico W em conjunto com a placa BitDogLab V6.3. Baseado na técnica Pomodoro, o projeto permite que o usuário configure intervalos de estudo e pausas, oferecendo feedback visual através de um display OLED (SSD1306) e uma matriz de 25 LEDs. Este sistema foi concebido para melhorar a produtividade, promovendo períodos de trabalho focado intercalados com breves intervalos de descanso.

Modo de Uso:
------------
1. **Inicialização:** Ao ligar o dispositivo, o sistema exibe uma mensagem de boas-vindas no display e apresenta uma animação na matriz de LEDs.
2. **Configuração dos Tempos:** Utilizando o joystick, o usuário seleciona o tempo de estudo (opções: 25, 30 ou 35 minutos) e o tempo de pausa (opções: 5, 10 ou 15 minutos). A escolha é exibida no display.
3. **Início do Ciclo:** Pressione o botão A para iniciar o ciclo. Durante o estudo, os LEDs se acendem sequencialmente para indicar o progresso do tempo.
4. **Transição entre Ciclos:** Ao final do ciclo de estudo, o sistema automaticamente passa para a fase de pausa, exibindo mensagens apropriadas e apagando a matriz de LEDs.
5. **Reinicialização:** Se necessário, o botão B reinicia o sistema (por meio de um watchdog), permitindo nova configuração dos tempos.

Componentes Utilizados:
------------------------
- **Raspberry Pi Pico/Pico W**
- **Placa BitDogLab V6.3**
- **Display OLED SSD1306**
- **Matriz de 25 LEDs**
- **Joystick e Botões (A, B e do joystick)**
- **ADC para leitura analógica do joystick**
- **Interface I²C para comunicação com o display**

Funcionamento e Estrutura do Firmware:
----------------------------------------
O firmware está estruturado em módulos que cuidam da inicialização do hardware, configuração dos tempos e gerenciamento dos ciclos de estudo/pausa. Durante a fase de estudo, um alarme distribui o tempo igualmente para acender os LEDs da matriz, oferecendo uma indicação visual do progresso. Quando os ciclos atingem seus limites (4 ciclos de estudo e 3 pausas), o sistema encerra a sessão e aguarda o reinício.


Autor:
------
Dorivaldo Jesus da Silva Júnior
