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
std::cout << "               \033[31m/######*\033[0m    @@@@@@@@.  ,@@@@@@@@,        Arthur, Guilherme e Júlia                                        " << std::endl;
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

