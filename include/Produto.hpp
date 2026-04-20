#ifndef PRODUTO_HPP
#define PRODUTO_HPP

#include "ItemEstoque.hpp"

class Produto : public ItemEstoque {
public:
    Produto(std::string codigo,
            std::string nome,
            int quantidade,
            double precoUnitario,
            std::string localizacao,
            std::string termoBusca);

    std::string tipo() const override;
    void imprimirResumo(std::ostream& saida) const override;
    void imprimirDetalhado(std::ostream& saida) const override;
};

#endif
