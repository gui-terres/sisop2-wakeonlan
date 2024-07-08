#include "subsystems/interface/interface.hpp"
#include "subsystems/discovery/discovery.hpp"

using namespace std;

int main(int argc, char **argv) {
    enableRawMode(); // Configura o terminal para o modo "raw"

    // Inicializa os botões
    initializeButtons();

    int selectedButton = 1; // Botão inicialmente selecionado
    bool isArrowKey_v = 0;
    bool isRawMode = 0;

    // Loop principal para interação com o usuário
    while (true) {
        // Desenha a interface com os botões e mostra no terminal
        if (isRawMode){
            drawInterface(selectedButton);
            printf("Digite 'BACK' para voltar ao menu.\n");
            string input;
            getline(cin, input);

            if (input == "EXIT"){
                selectedButton=4;
                break;
            }else if (input == "BACK"){
                enableRawMode();
                isRawMode = false;
            }else{
                continue;
            }
        }else{
            drawInterface(selectedButton);
            // Lê o caractere de entrada do usuário
            char input;
            std::cin >> input;

            bool tmp = isArrowKey(input);
            // Processa a entrada do usuário para atualizar o botão selecionado
            processInput(input, selectedButton, isArrowKey_v);

            isArrowKey_v = tmp;

            // Verifica se o usuário pressionou Enter para sair do programa
            if (input == '\n') {
                if (selectedButton == 3) {
                    disableRawMode();
                    isRawMode = true;
                    continue;   
                }
                break;
            }
        }
    }

    switch (selectedButton) {
        case 1: {
            cout << "Manager mode" << endl;
            Server server;
            server.sendSocket();
            break;
        }
        case 2: {
            //cout << "Client mode" << endl;
            showData();
            Client client;
            client.sendSocket(argc, argv[1]);
            break;
        }
        case 4: {
            printf("Saindo do sistema...\n");
            break;
        }
        default:
            cout << "Invalid button selected!" << endl;
            break;
    }

    disableRawMode(); // Restaura o terminal para as configurações originais

    return 0;
}
