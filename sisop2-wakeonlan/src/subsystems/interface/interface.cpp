#include "interface.hpp"


void clearScreen() {
    std::cout << "\033[2J\033[H"; // Código de escape ANSI para limpar a tela
}

void drawHeader() {
    std::cout << std::endl;
std::cout << "                                                                              *#&@@@@@@@%                                                     " << std::endl;
std::cout << "                                                                           #@@@@@@@@@@@@&                                                     " << std::endl;
std::cout << "                                                                         ,@@@@@@@@@@@@@@&                                                     " << std::endl;
std::cout << "                           &@@@@@@@.  ,@@@%/      ,#@@@@@@@@@@@%*        @@@@@@@@@/                                                           " << std::endl;
std::cout << "                           @@@@@@@@.  ,@@@@@@@. &@@@@@@@@@@@@@@@@@@.     @@@@@@@@@.                                                           " << std::endl;
std::cout << "                           @@@@@@@@.  ,@@@@@@@@@@@@@@@@@@@@@@@@@@@@@/    @@@@@@@@@@@@@@@&                                                     " << std::endl;
std::cout << "                           @@@@@@@@.  ,@@@@@@@@@@,        .@@@@@@@@@@.   @@@@@@@@@@@@@@@&                                                     " << std::endl;
std::cout << "                           @@@@@@@@.  ,@@@@@@@@&            @@@@@@@@@,   @@@@@@@@@*,,,,,.                                                     " << std::endl;
std::cout << "                           @@@@@@@@.  ,@@@@@@@@*            &@@@@@@@@*   @@@@@@@@@.                                                           " << std::endl;
std::cout << "                           @@@@@@@@.  ,@@@@@@@@,            &@@@@@@@@*   @@@@@@@@@.                                                           " << std::endl;
std::cout << "                           @@@@@@@@.  ,@@@@@@@@,            &@@@@@@@@*   @@@@@@@@@.                                                           " << std::endl;
std::cout << "                           @@@@@@@@.  ,@@@@@@@@,            %&&&&&&&&*   &&&&&&&&&.                                                           " << std::endl;
std::cout << "                 \033[31m*((*\033[0m      @@@@@@@@.  ,@@@@@@@@,      \033[30m...........................................\033[0m               " << std::endl;
std::cout << "              \033[31m*########,\033[0m   @@@@@@@@.  ,@@@@@@@@,        \033[31mINF01151 - Sistemas Operacionais Ii N - Turma A (2024/1)\033[0m" << std::endl;
std::cout << "              \033[31m(########/\033[0m   @@@@@@@@.  ,@@@@@@@@,        \033[31mTrabalho prático da disciplina\033[0m                          " << std::endl;
std::cout << "               \033[31m/######*\033[0m    @@@@@@@@.  ,@@@@@@@@,        Arthur, Cauê, Guilherme e Júlia                                        " << std::endl;
std::cout << std::endl;
std::cout << "                                                         _       __      __              ____              __                             " << std::endl;
std::cout << "                                                        | |     / /___ _/ /_____        / __ \\____        / /   ____ _____               " << std::endl;
std::cout << "                                                        | | /| / / __ `/ //_/ _ \\______/ / / / __ \\______/ /   / __ `/ __ \\            " << std::endl;
std::cout << "                                                        | |/ |/ / /_/ / ,< /  __/_____/ /_/ / / / /_____/ /___/ /_/ / / / /               " << std::endl;
std::cout << "                                                        |__/|__/\\__,_/_/|_|\\___/      \\____/_/ /_/     /_____/\\__,_/_/ /_/            " << std::endl;
std::cout << "                                                                              __           __                                             " << std::endl;
std::cout << "                                                            ____  ____ ______/ /____     <  /                                             " << std::endl;
std::cout << "                                                           / __ \\/ __ `/ ___/ __/ _ \\    / /                                            " << std::endl;
std::cout << "                                                          / /_/ / /_/ / /  / /_/  __/   / /                                               " << std::endl;
std::cout << "                                                         / .___/\\__,_/_/   \\__/\\___/   /_/                                             " << std::endl;
std::cout << "                                                        /_/                                                                               " << std::endl;
std::cout << std::endl;
std::cout << std::endl;

}
void drawInterface(){
    clearScreen(); 
    drawHeader();
}

// Função para configurar o terminal para ler uma tecla sem esperar por '\n'
void setTermNoBufferedInput() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO); // Desliga modo canônico e echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

// Função para restaurar as configurações originais do terminal
void restoreTermSettings() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= (ICANON | ECHO); // Liga modo canônico e echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

/*
void drawInterface(int selectedButton) {
    
    clearScreen(); // Limpa a tela antes de desenhar a interface
    // Define as cores ANSI para as cores de fundo e texto
    drawHeader();
    const std::string SELECTED_COLOR = "\033[48;2;220;50;50;38;2;0;0;0m"; // Fundo vermelho, texto preto
    const std::string DEFAULT_COLOR = "\033[49;39m"; // Fundo padrão, texto padrão
    const std::string SPACING = "    "; // Espaçamento entre os botões

    // Desenha os botões
    for (int i = 0; i <= (NUM_BUTTONS-1); ++i) {
        // Define o texto do botão e a cor de fundo
        std::string buttonText = (i == selectedButton - 1) ? SELECTED_COLOR + "[ " + buttons[i].label + " ]" + DEFAULT_COLOR : "[ " + buttons[i].label + " ]";

        // Imprime o botão na tela com o espaçamento
        std::cout << buttonText << SPACING;
    }
    // Mostra o cursor para indicar a posição do usuário
    std::cout << std::endl << std::endl << "Selecione um botão usando as teclas de seta ou WASD." << std::endl;
}

// Definição do ponteiro global para o array de botões
Button* buttons = nullptr;

// Função para inicializar os botões
void initializeButtons() {
    buttons = static_cast<Button*>(malloc(NUM_BUTTONS * sizeof(Button)));

    if (buttons == nullptr) {
        std::cerr << "Erro ao alocar memória para os botões!" << std::endl;
        exit(1);
    }

    new (&buttons[0]) Button{"Acordar uma máquina"};
    new (&buttons[1]) Button{"Listar máquinas"};
    new (&buttons[2]) Button{"Ativar linha de comando"};
    new (&buttons[3]) Button{"Sair"};
}


// Estrutura para armazenar as configurações originais do terminal
struct termios originalTermios;

void enableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &originalTermios); // Salva as configurações originais do terminal
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios); // Restaura as configurações originais do terminal
}

// Função para verificar se o caractere é uma seta do teclado
bool isArrowKey(char input) {
    // Caracteres de escape ANSI para setas do teclado
    const char ARROW_ESCAPE = '\x1b';
    const char LEFT_BRACKET = '[';

    // Verifica se o caractere é uma sequência de escape ANSI para setas
    if (input == ARROW_ESCAPE) {
        char nextInput;
        std::cin >> std::noskipws >> nextInput; // Lê o próximo caractere sem ignorar espaços em branco
        return (nextInput == LEFT_BRACKET);
    }

    return false;
}

void processInput(char input, int &selectedButton, bool isArrowKey) {
    // Verifica se o próximo caractere é uma sequência de escape para as teclas de seta
    char nextInput;

    if (isArrowKey) {        
        if (input == 'C' || input == 'A') {
            // Se pressionar a seta para a direita ou para cima
            if (selectedButton < NUM_BUTTONS) {
                selectedButton++;
            }
        } else if (input == 'D' || input == 'B') {
            // Se pressionar a seta para a esquerda ou para baixo
            if (selectedButton > 1) {
                selectedButton--;
            }
        }
    } else {
        // Atualiza o botão selecionado com base na entrada do usuário
        if (input == 'd' || input == 'D' || input == 'w' || input == 'W') {
            // Se pressionar 'd', 'D', 'w' ou 'W'
            if (selectedButton < NUM_BUTTONS) {
                selectedButton++;
            }
        } else if (input == 'a' || input == 'A' || input == 's' || input == 'S') {
            // Se pressionar 'a', 'A', 's' ou 'S'
            if (selectedButton > 1) {
                selectedButton--;
            }
        } else {
            
        }
    }
}
*/