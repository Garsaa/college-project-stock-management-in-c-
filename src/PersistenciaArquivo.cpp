#include "PersistenciaArquivo.hpp"

#include "Produto.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {

std::vector<std::string> dividirLinha(const std::string& linha) {
    std::vector<std::string> campos;
    std::stringstream entrada(linha);
    std::string campo;

    while (std::getline(entrada, campo, '|')) {
        campos.push_back(campo);
    }
    if (!linha.empty() && linha.back() == '|') {
        campos.push_back("");
    }

    return campos;
}

std::string juntarCampos(const std::vector<std::string>& campos) {
    std::ostringstream linha;

    for (std::size_t i = 0; i < campos.size(); ++i) {
        if (i > 0) {
            linha << '|';
        }
        linha << campos[i];
    }

    return linha.str();
}

}

PersistenciaArquivo::PersistenciaArquivo(std::string caminhoArquivo)
    : caminhoArquivo_(std::move(caminhoArquivo)) {}

void PersistenciaArquivo::salvar(const Estoque& estoque) const {
    const std::filesystem::path caminho(caminhoArquivo_);
    if (caminho.has_parent_path()) {
        std::filesystem::create_directories(caminho.parent_path());
    }

    std::ofstream arquivo(caminhoArquivo_);
    if (!arquivo) {
        throw std::runtime_error("Nao foi possivel abrir o arquivo para escrita: " + caminhoArquivo_);
    }

    arquivo << "# ITEM|codigo|tipo|nome|descricao|linkInformacoes|quantidade|preco|localizacao\n";
    arquivo << "# MOV|codigo|operacao|quantidade|observacao\n";
    for (const auto& item : estoque.itens()) {
        std::vector<std::string> campos = {
            "ITEM",
            item->codigo(),
            item->tipo(),
            item->nome(),
            item->descricao(),
            item->linkInformacoes(),
            std::to_string(item->quantidade()),
            std::to_string(item->precoUnitario()),
            item->localizacao()
        };

        arquivo << juntarCampos(campos) << '\n';

        for (const auto& movimento : item->movimentos()) {
            arquivo << juntarCampos({
                "MOV",
                item->codigo(),
                movimento.operacao,
                std::to_string(movimento.quantidade),
                movimento.observacao
            }) << '\n';
        }
    }
}

bool PersistenciaArquivo::carregar(Estoque& estoque) const {
    std::ifstream arquivo(caminhoArquivo_);
    if (!arquivo) {
        return false;
    }

    Estoque temporario;
    std::string linha;
    int numeroLinha = 0;

    while (std::getline(arquivo, linha)) {
        ++numeroLinha;
        if (linha.empty() || linha[0] == '#') {
            continue;
        }

        try {
            const auto campos = dividirLinha(linha);
            if (!campos.empty() && campos[0] == "MOV") {
                if (campos.size() < 5) {
                    throw std::runtime_error("movimento deve ter 5 campos");
                }

                auto* item = temporario.localizarPorCodigo(campos[1]);
                if (item == nullptr) {
                    throw std::runtime_error("movimento sem item correspondente");
                }

                item->adicionarRegistroMovimento(campos[2], std::stoi(campos[3]), campos[4]);
            } else {
                auto item = criarItem(campos);
                if (!temporario.adicionar(std::move(item))) {
                    throw std::runtime_error("codigo duplicado");
                }
            }
        } catch (const std::exception& erro) {
            std::ostringstream mensagem;
            mensagem << "Erro ao carregar linha " << numeroLinha << ": " << erro.what();
            throw std::runtime_error(mensagem.str());
        }
    }

    estoque = std::move(temporario);
    return true;
}

const std::string& PersistenciaArquivo::caminhoArquivo() const {
    return caminhoArquivo_;
}

std::unique_ptr<ItemEstoque> PersistenciaArquivo::criarItem(const std::vector<std::string>& campos) const {
    if (campos.empty()) {
        throw std::runtime_error("linha vazia");
    }

    if (campos[0] == "ITEM") {
        if (campos.size() < 9) {
            throw std::runtime_error("item deve ter 9 campos");
        }

        return std::make_unique<Produto>(
            campos[1],
            campos[2],
            campos[3],
            campos[4],
            campos[5],
            std::stoi(campos[6]),
            std::stod(campos[7]),
            campos[8]);
    }

    if (campos.size() < 5) {
        throw std::runtime_error("quantidade de campos insuficiente");
    }

    const auto& codigo = campos[0];
    const auto& nome = campos[1];
    const int quantidade = std::stoi(campos[2]);
    const double preco = std::stod(campos[3]);
    const auto& localizacao = campos[4];
    std::string linkInformacoes = campos.size() >= 6 ? campos[5] : "";
    if (linkInformacoes.empty()) {
        std::string nomeBusca = nome;
        for (auto& caractere : nomeBusca) {
            if (caractere == ' ') {
                caractere = '+';
            }
        }
        linkInformacoes = "https://www.google.com/search?tbm=isch&q=" + nomeBusca;
    }

    auto item = std::make_unique<Produto>(
        codigo,
        "produto",
        nome,
        "Descricao importada do formato antigo.",
        linkInformacoes,
        quantidade,
        preco,
        localizacao);

    if (quantidade > 0) {
        item->adicionarRegistroMovimento("Entrada", quantidade, "Importado do arquivo antigo.");
    }

    return item;
}
