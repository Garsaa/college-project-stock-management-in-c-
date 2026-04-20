#include "Produto.hpp"

#include "ConsoleUtils.hpp"

#include <iomanip>
#include <ostream>
#include <utility>

Produto::Produto(std::string codigo,
                 std::string tipoItem,
                 std::string nome,
                 std::string descricao,
                 std::string linkInformacoes,
                 int quantidade,
                 double precoUnitario,
                 std::string localizacao)
    : ItemEstoque(std::move(codigo),
                  std::move(tipoItem),
                  std::move(nome),
                  std::move(descricao),
                  std::move(linkInformacoes),
                  quantidade,
                  precoUnitario,
                  std::move(localizacao)) {}

std::string Produto::tipo() const {
    return tipoItem();
}

void Produto::imprimirResumo(std::ostream& saida) const {
    saida << std::left << std::setw(12) << codigo()
          << std::setw(12) << tipo()
          << std::setw(28) << nome().substr(0, 27)
          << std::right << std::setw(8) << quantidade()
          << std::setw(14) << console::formatarMoeda(precoUnitario())
          << std::setw(14) << console::formatarMoeda(valorTotal())
          << "  " << localizacao() << '\n';
}

void Produto::imprimirDetalhado(std::ostream& saida) const {
    saida << "Codigo: " << codigo() << '\n';
    saida << "Tipo: " << tipo() << '\n';
    saida << "Nome: " << nome() << '\n';
    saida << "Descricao: " << descricao() << '\n';
    saida << "Link de informacoes: " << linkInformacoes() << '\n';
    saida << "Quantidade: " << quantidade() << '\n';
    saida << "Preco unitario: R$ " << console::formatarMoeda(precoUnitario()) << '\n';
    saida << "Valor total: R$ " << console::formatarMoeda(valorTotal()) << '\n';
    saida << "Localizacao: " << localizacao() << '\n';

    saida << "Registros de entrada e saida:\n";
    if (movimentos().empty()) {
        saida << "  Nenhum movimento registrado.\n";
    } else {
        for (const auto& movimento : movimentos()) {
            saida << "  - " << movimento.operacao
                  << " | quantidade: " << movimento.quantidade;
            if (!movimento.observacao.empty()) {
                saida << " | " << movimento.observacao;
            }
            saida << '\n';
        }
    }
}
