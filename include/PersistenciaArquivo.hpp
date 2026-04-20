#ifndef PERSISTENCIA_ARQUIVO_HPP
#define PERSISTENCIA_ARQUIVO_HPP

#include "Estoque.hpp"

#include <memory>
#include <string>
#include <vector>

class PersistenciaArquivo {
public:
    explicit PersistenciaArquivo(std::string caminhoArquivo);

    void salvar(const Estoque& estoque) const;
    bool carregar(Estoque& estoque) const;
    const std::string& caminhoArquivo() const;

private:
    std::unique_ptr<ItemEstoque> criarItem(const std::vector<std::string>& campos) const;
    std::string caminhoArquivo_;
};

#endif
