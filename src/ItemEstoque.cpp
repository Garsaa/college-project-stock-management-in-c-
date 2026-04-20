#include "ItemEstoque.hpp"

#include <stdexcept>
#include <utility>

ItemEstoque::ItemEstoque(std::string codigo,
                         std::string nome,
                         int quantidade,
                         double precoUnitario,
                         std::string localizacao,
                         std::string termoBusca)
    : codigo_(std::move(codigo)),
      nome_(std::move(nome)),
      quantidade_(quantidade),
      precoUnitario_(precoUnitario),
      localizacao_(std::move(localizacao)),
      termoBusca_(std::move(termoBusca)) {
    validarCamposComuns();
}

const std::string& ItemEstoque::codigo() const {
    return codigo_;
}

const std::string& ItemEstoque::nome() const {
    return nome_;
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

const std::string& ItemEstoque::termoBusca() const {
    return termoBusca_;
}

void ItemEstoque::setNome(const std::string& nome) {
    if (nome.empty()) {
        throw std::invalid_argument("Nome nao pode ficar vazio.");
    }
    nome_ = nome;
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

void ItemEstoque::setTermoBusca(const std::string& termoBusca) {
    termoBusca_ = termoBusca;
}

double ItemEstoque::valorTotal() const {
    return quantidade_ * precoUnitario_;
}

std::string ItemEstoque::alvoBuscaInternet() const {
    return termoBusca_.empty() ? nome_ : termoBusca_;
}

void ItemEstoque::validarCamposComuns() const {
    if (codigo_.empty()) {
        throw std::invalid_argument("Codigo nao pode ficar vazio.");
    }
    if (nome_.empty()) {
        throw std::invalid_argument("Nome nao pode ficar vazio.");
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
