#ifndef ITEM_ESTOQUE_HPP
#define ITEM_ESTOQUE_HPP

#include <iosfwd>
#include <string>
#include <vector>

struct MovimentoEstoque {
    std::string operacao;
    int quantidade;
    std::string observacao;
};

class ItemEstoque {
public:
    ItemEstoque(std::string codigo,
                std::string tipoItem,
                std::string nome,
                std::string descricao,
                std::string linkInformacoes,
                int quantidade,
                double precoUnitario,
                std::string localizacao);
    virtual ~ItemEstoque() = default;

    const std::string& codigo() const;
    const std::string& tipoItem() const;
    const std::string& nome() const;
    const std::string& descricao() const;
    const std::string& linkInformacoes() const;
    int quantidade() const;
    double precoUnitario() const;
    const std::string& localizacao() const;
    const std::vector<MovimentoEstoque>& movimentos() const;

    void setTipoItem(const std::string& tipoItem);
    void setNome(const std::string& nome);
    void setDescricao(const std::string& descricao);
    void setLinkInformacoes(const std::string& linkInformacoes);
    void setQuantidade(int quantidade);
    void setPrecoUnitario(double precoUnitario);
    void setLocalizacao(const std::string& localizacao);

    double valorTotal() const;
    std::string alvoBuscaInternet() const;
    void registrarEntrada(int quantidade, const std::string& observacao);
    void registrarSaida(int quantidade, const std::string& observacao);
    void adicionarRegistroMovimento(const std::string& operacao,
                                    int quantidade,
                                    const std::string& observacao);

    virtual std::string tipo() const = 0;
    virtual void imprimirResumo(std::ostream& saida) const = 0;
    virtual void imprimirDetalhado(std::ostream& saida) const = 0;

protected:
    void validarCamposComuns() const;

private:
    std::string codigo_;
    std::string tipoItem_;
    std::string nome_;
    std::string descricao_;
    std::string linkInformacoes_;
    int quantidade_;
    double precoUnitario_;
    std::string localizacao_;
    std::vector<MovimentoEstoque> movimentos_;
};

#endif
