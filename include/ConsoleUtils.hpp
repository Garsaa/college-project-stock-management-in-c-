#ifndef CONSOLE_UTILS_HPP
#define CONSOLE_UTILS_HPP

#include <string>

namespace console {

std::string lerLinha(const std::string& mensagem);
std::string lerLinhaOpcional(const std::string& mensagem, const std::string& valorAtual);
int lerInteiro(const std::string& mensagem, int minimo = 0);
int lerInteiroOpcional(const std::string& mensagem, int valorAtual, int minimo = 0);
double lerDouble(const std::string& mensagem, double minimo = 0.0);
double lerDoubleOpcional(const std::string& mensagem, double valorAtual, double minimo = 0.0);
bool lerSimNao(const std::string& mensagem, bool valorPadrao = false);
void pausar();
void limparTela();
std::string paraMinusculo(std::string texto);
std::string formatarMoeda(double valor);

}

#endif
