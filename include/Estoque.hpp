#ifndef ESTOQUE_HPP
#define ESTOQUE_HPP

#include "ItemEstoque.hpp"

#include <memory>
#include <string>
#include <vector>

class Estoque {
public:
    bool adicionar(std::unique_ptr<ItemEstoque> item);
    bool removerPorCodigo(const std::string& codigo);
    ItemEstoque* localizarPorCodigo(const std::string& codigo);
    const ItemEstoque* localizarPorCodigo(const std::string& codigo) const;
    std::vector<ItemEstoque*> localizarPorNome(const std::string& termo);
    const std::vector<std::unique_ptr<ItemEstoque>>& itens() const;
    bool vazio() const;
    void limpar();

private:
    std::vector<std::unique_ptr<ItemEstoque>> itens_;
};

#endif
