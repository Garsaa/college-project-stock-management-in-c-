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

    arquivo << "# codigo|nome|quantidade|preco|localizacao|busca\n";
    for (const auto& item : estoque.itens()) {
        std::vector<std::string> campos = {
            item->codigo(),
            item->nome(),
            std::to_string(item->quantidade()),
            std::to_string(item->precoUnitario()),
            item->localizacao(),
            item->termoBusca()
        };

        arquivo << juntarCampos(campos) << '\n';
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
            auto item = criarItem(dividirLinha(linha));
            if (!temporario.adicionar(std::move(item))) {
                throw std::runtime_error("codigo duplicado");
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
    if (campos.size() < 5) {
        throw std::runtime_error("quantidade de campos insuficiente");
    }

    const auto& codigo = campos[0];
    const auto& nome = campos[1];
    const int quantidade = std::stoi(campos[2]);
    const double preco = std::stod(campos[3]);
    const auto& localizacao = campos[4];
    const std::string termoBusca = campos.size() >= 6 ? campos[5] : "";

    return std::make_unique<Produto>(
        codigo,
        nome,
        quantidade,
        preco,
        localizacao,
        termoBusca);
}
