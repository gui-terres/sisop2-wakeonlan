#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <termios.h>
#include <cstdlib> 
#include <unistd.h>

// Função para limpar a tela do terminal
void clearScreen();

// Função que desenha o banner inicial da aplicação.
void drawHeader();

// Função para desenhar a interface com os botões
void drawInterface();

// Função para configurar o terminal para ler os caracteres 
void setTermNoBufferedInput();

// Função para restaurar terminal
void restoreTermSettings();

#endif // INTERFACE_HPP
