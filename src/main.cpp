#include "ConsoleUtils.hpp"
#include "Estoque.hpp"
#include "PersistenciaArquivo.hpp"
#include "Produto.hpp"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

constexpr const char* CAMINHO_ARQUIVO = "data/estoque.txt";

std::string codificarUrl(const std::string& texto) {
    std::ostringstream saida;
    const char* hex = "0123456789ABCDEF";

    for (const unsigned char c : texto) {
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            saida << static_cast<char>(c);
        } else if (c == ' ') {
            saida << '+';
        } else {
            saida << '%' << hex[c >> 4] << hex[c & 15];
        }
    }

    return saida.str();
}

bool comecaComHttp(const std::string& texto) {
    const auto textoMinusculo = console::paraMinusculo(texto);
    return textoMinusculo.rfind("http://", 0) == 0 || textoMinusculo.rfind("https://", 0) == 0;
}

std::string montarUrlBusca(const ItemEstoque& item) {
    const auto alvo = item.alvoBuscaInternet();
    if (comecaComHttp(alvo)) {
        return alvo;
    }

    return "https://www.google.com/search?q=" + codificarUrl(alvo);
}

void abrirUrlNoNavegador(const std::string& url) {
#ifdef _WIN32
    const std::string comando = "start \"\" \"" + url + "\"";
#elif __APPLE__
    const std::string comando = "open \"" + url + "\"";
#else
    const std::string comando = "xdg-open \"" + url + "\"";
#endif

    const int resultado = std::system(comando.c_str());
    if (resultado != 0) {
        std::cout << "Nao foi possivel abrir automaticamente. Acesse manualmente:\n" << url << '\n';
    }
}

void imprimirCabecalhoListagem() {
    std::cout << std::left << std::setw(12) << "Codigo"
              << std::setw(30) << "Nome"
              << std::right << std::setw(8) << "Qtd"
              << std::setw(14) << "Preco"
              << std::setw(14) << "Total"
              << "  Localizacao\n";
    std::cout << std::string(92, '-') << '\n';
}

void listarItens(const Estoque& estoque) {
    if (estoque.vazio()) {
        std::cout << "Estoque vazio.\n";
        return;
    }

    double totalGeral = 0.0;
    imprimirCabecalhoListagem();

    for (const auto& item : estoque.itens()) {
        item->imprimirResumo(std::cout);
        totalGeral += item->valorTotal();
    }

    std::cout << std::string(92, '-') << '\n';
    std::cout << "Valor total em estoque: R$ " << console::formatarMoeda(totalGeral) << '\n';
}

void adicionarItem(Estoque& estoque) {
    const std::string codigo = console::lerLinha("Codigo: ");
    if (estoque.localizarPorCodigo(codigo) != nullptr) {
        std::cout << "Ja existe um item com esse codigo.\n";
        return;
    }

    const std::string nome = console::lerLinha("Nome: ");
    const int quantidade = console::lerInteiro("Quantidade: ", 0);
    const double preco = console::lerDouble("Preco unitario: R$ ", 0.0);
    const std::string localizacao = console::lerLinha("Localizacao no estoque: ");

    std::cout << "Termo de busca/link (ENTER para usar o nome do item): ";
    std::string termoBusca;
    std::getline(std::cin, termoBusca);

    auto novoItem = std::make_unique<Produto>(
        codigo,
        nome,
        quantidade,
        preco,
        localizacao,
        termoBusca);

    if (estoque.adicionar(std::move(novoItem))) {
        std::cout << "Item adicionado com sucesso.\n";
    } else {
        std::cout << "Nao foi possivel adicionar o item.\n";
    }
}

void apagarItem(Estoque& estoque) {
    const std::string codigo = console::lerLinha("Codigo do item a apagar: ");
    const auto* item = estoque.localizarPorCodigo(codigo);

    if (item == nullptr) {
        std::cout << "Item nao encontrado.\n";
        return;
    }

    item->imprimirDetalhado(std::cout);
    if (console::lerSimNao("Confirma a exclusao?", false) && estoque.removerPorCodigo(codigo)) {
        std::cout << "Item apagado.\n";
    } else {
        std::cout << "Exclusao cancelada.\n";
    }
}

void mostrarItem(const Estoque& estoque) {
    const std::string codigo = console::lerLinha("Codigo do item: ");
    const auto* item = estoque.localizarPorCodigo(codigo);

    if (item == nullptr) {
        std::cout << "Item nao encontrado.\n";
        return;
    }

    item->imprimirDetalhado(std::cout);
}

void localizarItem(Estoque& estoque) {
    std::cout << "1 - Localizar por codigo\n";
    std::cout << "2 - Localizar por nome\n";

    const int opcao = console::lerInteiro("Escolha: ", 1);
    if (opcao == 1) {
        mostrarItem(estoque);
        return;
    }

    if (opcao != 2) {
        std::cout << "Opcao invalida.\n";
        return;
    }

    const std::string termo = console::lerLinha("Parte do nome: ");
    const auto encontrados = estoque.localizarPorNome(termo);
    if (encontrados.empty()) {
        std::cout << "Nenhum item encontrado.\n";
        return;
    }

    imprimirCabecalhoListagem();
    for (const auto* item : encontrados) {
        item->imprimirResumo(std::cout);
    }
}

void modificarItem(Estoque& estoque) {
    const std::string codigo = console::lerLinha("Codigo do item a modificar: ");
    auto* item = estoque.localizarPorCodigo(codigo);

    if (item == nullptr) {
        std::cout << "Item nao encontrado.\n";
        return;
    }

    std::cout << "Valores atuais:\n";
    item->imprimirDetalhado(std::cout);
    std::cout << "\nDigite ENTER para manter o valor atual.\n";

    item->setNome(console::lerLinhaOpcional("Nome", item->nome()));
    item->setQuantidade(console::lerInteiroOpcional("Quantidade", item->quantidade(), 0));
    item->setPrecoUnitario(console::lerDoubleOpcional("Preco unitario", item->precoUnitario(), 0.0));
    item->setLocalizacao(console::lerLinhaOpcional("Localizacao", item->localizacao()));
    item->setTermoBusca(console::lerLinhaOpcional("Termo/link de busca", item->termoBusca()));

    std::cout << "Item modificado.\n";
}

void buscarNaInternet(Estoque& estoque) {
    const std::string codigo = console::lerLinha("Codigo do item para buscar na internet: ");
    const auto* item = estoque.localizarPorCodigo(codigo);

    if (item == nullptr) {
        std::cout << "Item nao encontrado.\n";
        return;
    }

    const auto url = montarUrlBusca(*item);
    std::cout << "Abrindo: " << url << '\n';
    abrirUrlNoNavegador(url);
}

void salvarItens(const PersistenciaArquivo& persistencia, const Estoque& estoque) {
    persistencia.salvar(estoque);
    std::cout << "Itens salvos em " << persistencia.caminhoArquivo() << ".\n";
}

void carregarItensAoIniciar(const PersistenciaArquivo& persistencia, Estoque& estoque) {
    if (persistencia.carregar(estoque)) {
        std::cout << "Itens carregados de " << persistencia.caminhoArquivo() << ".\n";
    } else {
        std::cout << "Arquivo ainda nao existe. Um novo estoque sera criado ao salvar.\n";
    }
}

void imprimirMenu() {
    std::cout << "\n==== Modulo de Estoque ====\n";
    std::cout << "1 - Adicionar item\n";
    std::cout << "2 - Apagar item\n";
    std::cout << "3 - Mostrar item\n";
    std::cout << "4 - Localizar item\n";
    std::cout << "5 - Modificar item\n";
    std::cout << "6 - Salvar itens em arquivo\n";
    std::cout << "7 - Imprimir listagem de itens\n";
    std::cout << "8 - Buscar item na internet\n";
    std::cout << "0 - Sair\n";
}

}

int main() {
    Estoque estoque;
    PersistenciaArquivo persistencia(CAMINHO_ARQUIVO);

    try {
        carregarItensAoIniciar(persistencia, estoque);
    } catch (const std::exception& erro) {
        std::cout << "Falha ao carregar arquivo inicial: " << erro.what() << '\n';
    }

    bool executando = true;
    while (executando) {
        try {
            imprimirMenu();
            const int opcao = console::lerInteiro("Escolha uma opcao: ", 0);

            switch (opcao) {
            case 1:
                adicionarItem(estoque);
                break;
            case 2:
                apagarItem(estoque);
                break;
            case 3:
                mostrarItem(estoque);
                break;
            case 4:
                localizarItem(estoque);
                break;
            case 5:
                modificarItem(estoque);
                break;
            case 6:
                salvarItens(persistencia, estoque);
                break;
            case 7:
                listarItens(estoque);
                break;
            case 8:
                buscarNaInternet(estoque);
                break;
            case 0:
                if (console::lerSimNao("Salvar antes de sair?", true)) {
                    salvarItens(persistencia, estoque);
                }
                executando = false;
                break;
            default:
                std::cout << "Opcao invalida.\n";
                break;
            }
        } catch (const std::exception& erro) {
            std::cout << "Erro: " << erro.what() << '\n';
        }
    }

    std::cout << "Programa encerrado.\n";
    return 0;
}
