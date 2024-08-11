#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include "../../stations/stations.hpp"
#include "../monitoring/monitoring.hpp"

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

void drawTableHeader();

void drawTableData(Server &server);

void drawTable(Server &server);

void clear_line();

void manipulateInput(char input[100], Client &client, Server &server);

void read_input(Client &client, Server &server);

void handleSigInt(int signum);

void isCTRLc();

void isCTRLcT(Client &client);

#endif // INTERFACE_HPP
