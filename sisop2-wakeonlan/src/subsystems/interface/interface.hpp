#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <termios.h>
#include <cstdlib> // Para a função exit()
#include <unistd.h>
#include <vector>

#define NUM_BUTTONS 4
#define BUTTON_SIZE 20

// Estrutura para armazenar informações de cada botão
struct Button {
    std::string label;

    // Construtor para inicializar o botão
    Button(const std::string& lbl = "")
        : label(lbl) {}
};

struct info {
    std::string hostname;
    std::string macAddress;
    std::string ipAddress;
    std::string status;
};

// Função para inicializar os botões
void initializeButtons();

// Declaração do array global de botões
extern Button* buttons;

// Função para limpar a tela do terminal
void clearScreen();

// Função que desenha o banner inicial da aplicação.
void drawHeader();

// Função para desenhar a interface com os botões
void drawInterface(int selectedButton);

// Função para configurar o terminal para o modo "raw"
void enableRawMode();

// Função para restaurar as configurações originais do terminal
void disableRawMode();

// Função para processar a entrada do usuário e atualizar o botão selecionado
void processInput(char input, int &selectedButton, bool isArrowKey);

// Função para verificar se o caractere é uma seta do teclado
bool isArrowKey(char input);

void showData();

#endif // INTERFACE_HPP
