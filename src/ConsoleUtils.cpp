#include "ConsoleUtils.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace {

std::string trim(const std::string& texto) {
    const auto inicio = texto.find_first_not_of(" \t\r\n");
    if (inicio == std::string::npos) {
        return "";
    }

    const auto fim = texto.find_last_not_of(" \t\r\n");
    return texto.substr(inicio, fim - inicio + 1);
}

}

namespace console {

std::string lerLinha(const std::string& mensagem) {
    std::string valor;

    do {
        std::cout << mensagem;
        std::getline(std::cin, valor);
        valor = trim(valor);

        if (valor.empty()) {
            std::cout << "Valor obrigatorio. Tente novamente.\n";
        }
    } while (valor.empty());

    return valor;
}

std::string lerLinhaOpcional(const std::string& mensagem, const std::string& valorAtual) {
    std::cout << mensagem << " [" << valorAtual << "]: ";

    std::string valor;
    std::getline(std::cin, valor);
    valor = trim(valor);

    return valor.empty() ? valorAtual : valor;
}

int lerInteiro(const std::string& mensagem, int minimo) {
    while (true) {
        std::cout << mensagem;

        std::string entrada;
        std::getline(std::cin, entrada);
        std::stringstream ss(trim(entrada));

        int valor = 0;
        char resto = '\0';
        if (ss >> valor && !(ss >> resto) && valor >= minimo) {
            return valor;
        }

        std::cout << "Informe um numero inteiro maior ou igual a " << minimo << ".\n";
    }
}

int lerInteiroOpcional(const std::string& mensagem, int valorAtual, int minimo) {
    while (true) {
        std::cout << mensagem << " [" << valorAtual << "]: ";

        std::string entrada;
        std::getline(std::cin, entrada);
        entrada = trim(entrada);

        if (entrada.empty()) {
            return valorAtual;
        }

        std::stringstream ss(entrada);
        int valor = 0;
        char resto = '\0';
        if (ss >> valor && !(ss >> resto) && valor >= minimo) {
            return valor;
        }

        std::cout << "Informe um numero inteiro maior ou igual a " << minimo << ".\n";
    }
}

double lerDouble(const std::string& mensagem, double minimo) {
    while (true) {
        std::cout << mensagem;

        std::string entrada;
        std::getline(std::cin, entrada);
        std::replace(entrada.begin(), entrada.end(), ',', '.');
        std::stringstream ss(trim(entrada));

        double valor = 0.0;
        char resto = '\0';
        if (ss >> valor && !(ss >> resto) && valor >= minimo) {
            return valor;
        }

        std::cout << "Informe um valor numerico maior ou igual a " << minimo << ".\n";
    }
}

double lerDoubleOpcional(const std::string& mensagem, double valorAtual, double minimo) {
    while (true) {
        std::cout << mensagem << " [" << formatarMoeda(valorAtual) << "]: ";

        std::string entrada;
        std::getline(std::cin, entrada);
        entrada = trim(entrada);

        if (entrada.empty()) {
            return valorAtual;
        }

        std::replace(entrada.begin(), entrada.end(), ',', '.');
        std::stringstream ss(entrada);

        double valor = 0.0;
        char resto = '\0';
        if (ss >> valor && !(ss >> resto) && valor >= minimo) {
            return valor;
        }

        std::cout << "Informe um valor numerico maior ou igual a " << minimo << ".\n";
    }
}

bool lerSimNao(const std::string& mensagem, bool valorPadrao) {
    while (true) {
        std::cout << mensagem << (valorPadrao ? " [S/n]: " : " [s/N]: ");

        std::string entrada;
        std::getline(std::cin, entrada);
        entrada = paraMinusculo(trim(entrada));

        if (entrada.empty()) {
            return valorPadrao;
        }
        if (entrada == "s" || entrada == "sim") {
            return true;
        }
        if (entrada == "n" || entrada == "nao") {
            return false;
        }

        std::cout << "Responda com s ou n.\n";
    }
}

void pausar() {
    std::cout << "\nPressione ENTER para continuar...";
    std::string descarte;
    std::getline(std::cin, descarte);
}

void limparTela() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

std::string paraMinusculo(std::string texto) {
    std::transform(texto.begin(), texto.end(), texto.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return texto;
}

std::string formatarMoeda(double valor) {
    std::ostringstream saida;
    saida << std::fixed << std::setprecision(2) << valor;
    return saida.str();
}

}
