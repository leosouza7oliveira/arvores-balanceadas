// =====================================================================
// Amazon Engineering Challenge - Arvores Balanceadas (AVL e Rubro-Negra)
// Trabalho da Unidade 2 - Estruturas de Dados Basicas II (IMD0039)
//
// Prototipo do sistema de logistica da Black Friday: guarda os IDs das
// encomendas numa arvore balanceada. O usuario escolhe a arvore (AVL ou
// Rubro-Negra) quando o programa abre e pode trocar depois pelo menu.
//
// Compilar:  make            (ou o comando g++ que esta no README)
// Executar:  ./bin/arvores   (use --sem-cor se o terminal nao mostrar cor)
//            ./bin/arvores --eco < exemplos/demo_avl.txt   (roda um roteiro pronto)
// =====================================================================

#include <cctype>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "avl.h"
#include "rubronegra.h"

using namespace std;

bool usarCores = true;
bool eco = false; // --eco: repete o que foi lido (util quando a entrada vem de arquivo)

// Le uma linha inteira. Retorna false se acabou a entrada (Ctrl+D ou
// fim do arquivo quando a entrada vem de um redirecionamento).
bool lerLinha(const string &msg, string &linha) {
    cout << msg;
    if (!getline(cin, linha)) {
        cout << "\n";
        return false;
    }
    if (eco) cout << linha << "\n";
    return true;
}

// Converte texto para int. Recusa coisas como "12abc" ou numero grande demais.
bool paraInteiro(const string &texto, int &valor) {
    try {
        size_t pos = 0;
        long long v = stoll(texto, &pos);
        while (pos < texto.size() && isspace((unsigned char)texto[pos])) pos++;
        if (pos != texto.size()) return false;
        if (v < INT32_MIN || v > INT32_MAX) return false;
        valor = (int)v;
        return true;
    } catch (...) {
        return false;
    }
}

// Pede um ID ate o usuario digitar um numero valido
bool lerID(const string &msg, int &id) {
    string linha;
    while (lerLinha(msg, linha)) {
        if (paraInteiro(linha, id)) return true;
        cout << "  Valor invalido, digite um numero inteiro.\n";
    }
    return false;
}

unique_ptr<ArvoreBusca> criarArvore(int tipo) {
    if (tipo == 1) return unique_ptr<ArvoreBusca>(new AVL());
    return unique_ptr<ArvoreBusca>(new RubroNegra());
}

// Mostra o que a ultima operacao custou em termos de balanceamento
void mostrarAjustes(const ArvoreBusca &arv, const Estatisticas &antes) {
    const Estatisticas &d = arv.estatisticas();
    long long rot = d.rotacoes - antes.rotacoes;
    if (arv.nome() == "AVL") {
        long long duplas = d.rotacoesDuplas - antes.rotacoesDuplas;
        cout << "  Rotacoes nesta operacao: " << rot;
        if (duplas > 0) cout << " (" << duplas << " dupla(s))";
        cout << "\n";
    } else {
        cout << "  Rotacoes nesta operacao: " << rot
             << " | Recoloracoes: " << d.recoloracoes - antes.recoloracoes << "\n";
    }
}

void mostrarEstatisticas(const ArvoreBusca &arv) {
    const Estatisticas &e = arv.estatisticas();
    cout << "  Estrutura ........: " << arv.nome() << "\n";
    cout << "  Elementos ........: " << arv.tamanho() << "\n";
    cout << "  Altura ...........: " << arv.altura() << "\n";
    cout << "  Rotacoes (total) .: " << e.rotacoes << "\n";
    if (arv.nome() == "AVL") {
        cout << "  Rotacoes duplas ..: " << e.rotacoesDuplas << "\n";
    } else {
        const RubroNegra &rb = static_cast<const RubroNegra &>(arv);
        cout << "  Recoloracoes .....: " << e.recoloracoes << "\n";
        cout << "  Altura negra .....: " << rb.alturaNegra() << "\n";
    }
    cout << "  Comparacoes em buscas: " << e.comparacoes << "\n";
}

void verificarArvore(const ArvoreBusca &arv) {
    string erro;
    if (arv.verificar(erro))
        cout << "  OK: todas as propriedades da arvore " << arv.nome() << " estao validas.\n";
    else
        cout << "  ERRO: " << erro << "\n";
}

int escolherTipo() {
    cout << "\nEscolha a estrutura de dados:\n";
    cout << "  1 - Arvore AVL\n";
    cout << "  2 - Arvore Rubro-Negra\n";
    cout << "  0 - Sair\n";
    string linha;
    int op;
    while (lerLinha("> ", linha)) {
        if (paraInteiro(linha, op) && op >= 0 && op <= 2) return op;
        cout << "  Opcao invalida.\n";
    }
    return 0;
}

void mostrarMenu(const ArvoreBusca &arv) {
    cout << "\n------ Arvore " << arv.nome() << " (" << arv.tamanho()
         << " elementos, altura " << arv.altura() << ") ------\n";
    cout << "  1 - Inserir encomenda (ID)\n";
    cout << "  2 - Remover encomenda\n";
    cout << "  3 - Buscar encomenda\n";
    cout << "  4 - Listar IDs em ordem crescente\n";
    cout << "  5 - Mostrar estrutura da arvore\n";
    cout << "  6 - Inserir varios IDs de uma vez\n";
    cout << "  7 - Estatisticas (altura, rotacoes...)\n";
    cout << "  8 - Verificar propriedades da arvore\n";
    cout << "  9 - Trocar tipo de arvore\n";
    cout << "  0 - Sair\n";
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++)
        if (string(argv[i]) == "--sem-cor") usarCores = false;
        else if (string(argv[i]) == "--eco") eco = true;

    cout << "==============================================\n";
    cout << "  Sistema de Encomendas - Black Friday\n";
    cout << "  Prototipo com arvores AVL e Rubro-Negra\n";
    cout << "==============================================\n";

    int tipo = escolherTipo();
    if (tipo == 0) return 0;
    unique_ptr<ArvoreBusca> arv = criarArvore(tipo);

    string linha;
    while (true) {
        mostrarMenu(*arv);
        if (!lerLinha("> ", linha)) break;
        int op;
        if (!paraInteiro(linha, op)) {
            cout << "  Opcao invalida.\n";
            continue;
        }

        if (op == 0) break;

        int id;
        Estatisticas antes = arv->estatisticas();
        switch (op) {
        case 1:
            if (!lerID("ID da encomenda: ", id)) return 0;
            if (arv->inserir(id)) {
                cout << "  Encomenda " << id << " registrada.\n";
                mostrarAjustes(*arv, antes);
            } else {
                cout << "  A encomenda " << id << " ja estava registrada. Nada foi alterado.\n";
            }
            break;

        case 2:
            if (!lerID("ID para remover: ", id)) return 0;
            if (arv->remover(id)) {
                cout << "  Encomenda " << id << " removida.\n";
                mostrarAjustes(*arv, antes);
            } else {
                cout << "  Encomenda " << id << " nao encontrada. Nada foi removido.\n";
            }
            break;

        case 3:
            if (!lerID("ID para buscar: ", id)) return 0;
            if (arv->buscar(id))
                cout << "  Encomenda " << id << " encontrada";
            else
                cout << "  Encomenda " << id << " NAO encontrada";
            cout << " (" << arv->estatisticas().comparacoes - antes.comparacoes
                 << " comparacoes).\n";
            break;

        case 4: {
            vector<int> v = arv->emOrdem();
            if (v.empty()) {
                cout << "  (arvore vazia)\n";
                break;
            }
            cout << "  ";
            for (size_t i = 0; i < v.size(); i++) cout << v[i] << (i + 1 < v.size() ? " " : "\n");
            break;
        }

        case 5:
            if (arv->tamanho() > 200)
                cout << "  Arvore grande (" << arv->tamanho() << " nos), a impressao pode ficar longa.\n";
            arv->imprimir(cout, usarCores);
            break;

        case 6: {
            if (!lerLinha("IDs separados por espaco ou virgula: ", linha)) return 0;
            for (char &c : linha)
                if (c == ',' || c == ';') c = ' ';
            stringstream ss(linha);
            string token;
            int ok = 0, repetidos = 0, invalidos = 0;
            while (ss >> token) {
                if (!paraInteiro(token, id)) {
                    invalidos++;
                    continue;
                }
                if (arv->inserir(id))
                    ok++;
                else
                    repetidos++;
            }
            cout << "  Inseridos: " << ok << " | Repetidos ignorados: " << repetidos
                 << " | Invalidos: " << invalidos << "\n";
            mostrarAjustes(*arv, antes);
            break;
        }

        case 7:
            mostrarEstatisticas(*arv);
            break;

        case 8:
            verificarArvore(*arv);
            break;

        case 9: {
            int novo = escolherTipo();
            if (novo == 0) break;
            vector<int> dados = arv->emOrdem();
            arv = criarArvore(novo);
            if (!dados.empty()) {
                string resp;
                if (lerLinha("Copiar os " + to_string(dados.size()) +
                                 " IDs atuais para a nova arvore? (s/n): ", resp) &&
                    !resp.empty() && (resp[0] == 's' || resp[0] == 'S')) {
                    for (int x : dados) arv->inserir(x);
                    arv->zerarEstatisticas();
                    cout << "  IDs copiados.\n";
                }
            }
            break;
        }

        default:
            cout << "  Opcao invalida.\n";
        }
    }

    cout << "Encerrando.\n";
    return 0;
}
