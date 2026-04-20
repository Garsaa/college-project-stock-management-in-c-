#include "Estoque.hpp"

#include "ConsoleUtils.hpp"

#include <algorithm>

bool Estoque::adicionar(std::unique_ptr<ItemEstoque> item) {
    if (!item || localizarPorCodigo(item->codigo()) != nullptr) {
        return false;
    }

    itens_.push_back(std::move(item));
    return true;
}

bool Estoque::removerPorCodigo(const std::string& codigo) {
    const auto posicao = std::find_if(itens_.begin(), itens_.end(), [&](const auto& item) {
        return item->codigo() == codigo;
    });

    if (posicao == itens_.end()) {
        return false;
    }

    itens_.erase(posicao);
    return true;
}

ItemEstoque* Estoque::localizarPorCodigo(const std::string& codigo) {
    const auto posicao = std::find_if(itens_.begin(), itens_.end(), [&](const auto& item) {
        return item->codigo() == codigo;
    });

    return posicao == itens_.end() ? nullptr : posicao->get();
}

const ItemEstoque* Estoque::localizarPorCodigo(const std::string& codigo) const {
    const auto posicao = std::find_if(itens_.begin(), itens_.end(), [&](const auto& item) {
        return item->codigo() == codigo;
    });

    return posicao == itens_.end() ? nullptr : posicao->get();
}

std::vector<ItemEstoque*> Estoque::localizarPorNome(const std::string& termo) {
    std::vector<ItemEstoque*> encontrados;
    const auto termoMinusculo = console::paraMinusculo(termo);

    for (const auto& item : itens_) {
        const auto nomeMinusculo = console::paraMinusculo(item->nome());
        if (nomeMinusculo.find(termoMinusculo) != std::string::npos) {
            encontrados.push_back(item.get());
        }
    }

    return encontrados;
}

const std::vector<std::unique_ptr<ItemEstoque>>& Estoque::itens() const {
    return itens_;
}

bool Estoque::vazio() const {
    return itens_.empty();
}

void Estoque::limpar() {
    itens_.clear();
}
