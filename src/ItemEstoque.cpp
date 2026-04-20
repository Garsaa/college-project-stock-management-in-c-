#include "ItemEstoque.hpp"

#include "ConsoleUtils.hpp"

#include <stdexcept>
#include <utility>

ItemEstoque::ItemEstoque(std::string codigo,
                         std::string tipoItem,
                         std::string nome,
                         std::string descricao,
                         std::string linkInformacoes,
                         int quantidade,
                         double precoUnitario,
                         std::string localizacao)
    : codigo_(std::move(codigo)),
      tipoItem_(std::move(tipoItem)),
      nome_(std::move(nome)),
      descricao_(std::move(descricao)),
      linkInformacoes_(std::move(linkInformacoes)),
      quantidade_(quantidade),
      precoUnitario_(precoUnitario),
      localizacao_(std::move(localizacao)) {
    tipoItem_ = console::paraMinusculo(tipoItem_);
    validarCamposComuns();
}

const std::string& ItemEstoque::codigo() const {
    return codigo_;
}

const std::string& ItemEstoque::tipoItem() const {
    return tipoItem_;
}

const std::string& ItemEstoque::nome() const {
    return nome_;
}

const std::string& ItemEstoque::descricao() const {
    return descricao_;
}

const std::string& ItemEstoque::linkInformacoes() const {
    return linkInformacoes_;
}

int ItemEstoque::quantidade() const {
    return quantidade_;
}

double ItemEstoque::precoUnitario() const {
    return precoUnitario_;
}

const std::string& ItemEstoque::localizacao() const {
    return localizacao_;
}

const std::vector<MovimentoEstoque>& ItemEstoque::movimentos() const {
    return movimentos_;
}

void ItemEstoque::setTipoItem(const std::string& tipoItem) {
    const auto tipoMinusculo = console::paraMinusculo(tipoItem);
    if (tipoMinusculo != "produto" && tipoMinusculo != "material") {
        throw std::invalid_argument("Tipo deve ser produto ou material.");
    }
    tipoItem_ = tipoMinusculo;
}

void ItemEstoque::setNome(const std::string& nome) {
    if (nome.empty()) {
        throw std::invalid_argument("Nome nao pode ficar vazio.");
    }
    nome_ = nome;
}

void ItemEstoque::setDescricao(const std::string& descricao) {
    if (descricao.empty()) {
        throw std::invalid_argument("Descricao nao pode ficar vazia.");
    }
    descricao_ = descricao;
}

void ItemEstoque::setLinkInformacoes(const std::string& linkInformacoes) {
    if (linkInformacoes.empty()) {
        throw std::invalid_argument("Link de informacoes nao pode ficar vazio.");
    }
    linkInformacoes_ = linkInformacoes;
}

void ItemEstoque::setQuantidade(int quantidade) {
    if (quantidade < 0) {
        throw std::invalid_argument("Quantidade nao pode ser negativa.");
    }
    quantidade_ = quantidade;
}

void ItemEstoque::setPrecoUnitario(double precoUnitario) {
    if (precoUnitario < 0.0) {
        throw std::invalid_argument("Preco nao pode ser negativo.");
    }
    precoUnitario_ = precoUnitario;
}

void ItemEstoque::setLocalizacao(const std::string& localizacao) {
    if (localizacao.empty()) {
        throw std::invalid_argument("Localizacao nao pode ficar vazia.");
    }
    localizacao_ = localizacao;
}

double ItemEstoque::valorTotal() const {
    return quantidade_ * precoUnitario_;
}

std::string ItemEstoque::alvoBuscaInternet() const {
    return linkInformacoes_.empty() ? nome_ : linkInformacoes_;
}

void ItemEstoque::registrarEntrada(int quantidade, const std::string& observacao) {
    if (quantidade <= 0) {
        throw std::invalid_argument("Quantidade de entrada deve ser maior que zero.");
    }

    quantidade_ += quantidade;
    adicionarRegistroMovimento("Entrada", quantidade, observacao);
}

void ItemEstoque::registrarSaida(int quantidade, const std::string& observacao) {
    if (quantidade <= 0) {
        throw std::invalid_argument("Quantidade de saida deve ser maior que zero.");
    }
    if (quantidade > quantidade_) {
        throw std::invalid_argument("Nao ha quantidade suficiente em estoque.");
    }

    quantidade_ -= quantidade;
    adicionarRegistroMovimento("Saida", quantidade, observacao);
}

void ItemEstoque::adicionarRegistroMovimento(const std::string& operacao,
                                             int quantidade,
                                             const std::string& observacao) {
    if (operacao != "Entrada" && operacao != "Saida") {
        throw std::invalid_argument("Operacao deve ser Entrada ou Saida.");
    }
    if (quantidade <= 0) {
        throw std::invalid_argument("Quantidade do movimento deve ser maior que zero.");
    }

    movimentos_.push_back({operacao, quantidade, observacao});
}

void ItemEstoque::validarCamposComuns() const {
    if (codigo_.empty()) {
        throw std::invalid_argument("Codigo nao pode ficar vazio.");
    }
    const auto tipoMinusculo = console::paraMinusculo(tipoItem_);
    if (tipoMinusculo != "produto" && tipoMinusculo != "material") {
        throw std::invalid_argument("Tipo deve ser produto ou material.");
    }
    if (nome_.empty()) {
        throw std::invalid_argument("Nome nao pode ficar vazio.");
    }
    if (descricao_.empty()) {
        throw std::invalid_argument("Descricao nao pode ficar vazia.");
    }
    if (linkInformacoes_.empty()) {
        throw std::invalid_argument("Link de informacoes nao pode ficar vazio.");
    }
    if (quantidade_ < 0) {
        throw std::invalid_argument("Quantidade nao pode ser negativa.");
    }
    if (precoUnitario_ < 0.0) {
        throw std::invalid_argument("Preco nao pode ser negativo.");
    }
    if (localizacao_.empty()) {
        throw std::invalid_argument("Localizacao nao pode ficar vazia.");
    }
}
