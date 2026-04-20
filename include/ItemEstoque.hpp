#ifndef ITEM_ESTOQUE_HPP
#define ITEM_ESTOQUE_HPP

#include <iosfwd>
#include <string>

class ItemEstoque {
public:
    ItemEstoque(std::string codigo,
                std::string nome,
                int quantidade,
                double precoUnitario,
                std::string localizacao,
                std::string termoBusca);
    virtual ~ItemEstoque() = default;

    const std::string& codigo() const;
    const std::string& nome() const;
    int quantidade() const;
    double precoUnitario() const;
    const std::string& localizacao() const;
    const std::string& termoBusca() const;

    void setNome(const std::string& nome);
    void setQuantidade(int quantidade);
    void setPrecoUnitario(double precoUnitario);
    void setLocalizacao(const std::string& localizacao);
    void setTermoBusca(const std::string& termoBusca);

    double valorTotal() const;
    std::string alvoBuscaInternet() const;

    virtual std::string tipo() const = 0;
    virtual void imprimirResumo(std::ostream& saida) const = 0;
    virtual void imprimirDetalhado(std::ostream& saida) const = 0;

protected:
    void validarCamposComuns() const;

private:
    std::string codigo_;
    std::string nome_;
    int quantidade_;
    double precoUnitario_;
    std::string localizacao_;
    std::string termoBusca_;
};

#endif
