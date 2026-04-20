#include "Produto.hpp"

#include "ConsoleUtils.hpp"

#include <iomanip>
#include <ostream>
#include <utility>

Produto::Produto(std::string codigo,
                 std::string nome,
                 int quantidade,
                 double precoUnitario,
                 std::string localizacao,
                 std::string termoBusca)
    : ItemEstoque(std::move(codigo),
                  std::move(nome),
                  quantidade,
                  precoUnitario,
                  std::move(localizacao),
                  std::move(termoBusca)) {}

std::string Produto::tipo() const {
    return "Produto";
}

void Produto::imprimirResumo(std::ostream& saida) const {
    saida << std::left << std::setw(12) << codigo()
          << std::setw(30) << nome().substr(0, 29)
          << std::right << std::setw(8) << quantidade()
          << std::setw(14) << console::formatarMoeda(precoUnitario())
          << std::setw(14) << console::formatarMoeda(valorTotal())
          << "  " << localizacao() << '\n';
}

void Produto::imprimirDetalhado(std::ostream& saida) const {
    saida << "Codigo: " << codigo() << '\n';
    saida << "Nome: " << nome() << '\n';
    saida << "Quantidade: " << quantidade() << '\n';
    saida << "Preco unitario: R$ " << console::formatarMoeda(precoUnitario()) << '\n';
    saida << "Valor total: R$ " << console::formatarMoeda(valorTotal()) << '\n';
    saida << "Localizacao: " << localizacao() << '\n';
    saida << "Busca/link: " << alvoBuscaInternet() << '\n';
}
