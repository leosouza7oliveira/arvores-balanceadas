// =====================================================================
// Testes automaticos das arvores AVL e Rubro-Negra
//
// Roda exatamente os mesmos cenarios nas duas arvores e confere, depois
// de cada insercao/remocao, se as propriedades continuam valendo
// (usando o metodo verificar() de cada arvore).
//
// Executar: ./bin/testes
// =====================================================================

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "avl.h"
#include "rubronegra.h"

using namespace std;

int totalTestes = 0;
int falhas = 0;

void checar(bool condicao, const string &descricao) {
    totalTestes++;
    if (condicao) {
        cout << "  [OK]    " << descricao << "\n";
    } else {
        falhas++;
        cout << "  [FALHA] " << descricao << "\n";
    }
}

// Retorna true se a arvore continua valida. Se nao, mostra o erro.
bool valida(const ArvoreBusca &arv) {
    string erro;
    if (arv.verificar(erro)) return true;
    cout << "          erro: " << erro << "\n";
    return false;
}

bool ordenadoSemRepeticao(const vector<int> &v) {
    for (size_t i = 1; i < v.size(); i++)
        if (v[i - 1] >= v[i]) return false;
    return true;
}

// Insere a sequencia verificando a arvore a cada passo
bool inserirVerificando(ArvoreBusca &arv, const vector<int> &seq) {
    for (int x : seq) {
        if (!arv.inserir(x)) return false;
        if (!valida(arv)) return false;
    }
    return true;
}

vector<int> sequencia(int n, const string &tipo, unsigned semente = 42) {
    vector<int> v(n);
    iota(v.begin(), v.end(), 1);
    if (tipo == "decrescente") reverse(v.begin(), v.end());
    if (tipo == "aleatoria") {
        mt19937 rng(semente);
        shuffle(v.begin(), v.end(), rng);
    }
    return v;
}

// ---------------------------------------------------------------------
// Cenarios
// ---------------------------------------------------------------------

void testeArvoreVazia(ArvoreBusca &arv) {
    cout << "\n> Arvore vazia\n";
    checar(arv.vazia() && arv.altura() == 0, "comeca vazia, com altura 0");
    checar(!arv.buscar(10), "buscar em arvore vazia retorna nao encontrado");
    checar(!arv.remover(10), "remover de arvore vazia retorna false");
    checar(arv.emOrdem().empty(), "percurso em ordem vazio");
    checar(valida(arv), "arvore vazia e valida");
}

void testeUmElemento(ArvoreBusca &arv) {
    cout << "\n> Arvore com um elemento\n";
    checar(arv.inserir(42), "inserir 42");
    int r = 0;
    checar(arv.chaveRaiz(r) && r == 42 && arv.altura() == 1, "42 virou a raiz e altura = 1");
    checar(arv.buscar(42) && !arv.buscar(7), "busca 42 (existe) e 7 (nao existe)");
    checar(valida(arv), "propriedades validas com 1 elemento");
    checar(!arv.remover(7), "remover 7 (inexistente) nao muda nada");
    checar(arv.tamanho() == 1, "continua com 1 elemento");
    checar(arv.remover(42) && arv.vazia(), "remover 42 deixa a arvore vazia");
    checar(valida(arv), "arvore vazia de novo e valida");
}

void testeRepetidos(ArvoreBusca &arv) {
    cout << "\n> Valores repetidos\n";
    arv.inserir(10);
    arv.inserir(20);
    checar(!arv.inserir(10), "inserir 10 de novo e recusado");
    checar(!arv.inserir(20), "inserir 20 de novo e recusado");
    checar(arv.tamanho() == 2 && arv.emOrdem() == vector<int>({10, 20}), "a arvore continua com {10, 20}");
    checar(valida(arv), "propriedades validas");
    arv.limpar();
}

// Os quatro casos de desbalanceamento com 3 elementos. Nas duas
// arvores o resultado tem que ser 20 na raiz.
void testeCasosRotacao(ArvoreBusca &arv) {
    cout << "\n> Casos basicos de rotacao (3 elementos)\n";
    vector<vector<int>> casos = {{10, 20, 30}, {30, 20, 10}, {10, 30, 20}, {30, 10, 20}};
    vector<string> nomes = {"crescente (RR)", "decrescente (LL)", "zigue-zague (RL)", "zigue-zague (LR)"};
    for (size_t i = 0; i < casos.size(); i++) {
        arv.limpar();
        arv.zerarEstatisticas();
        for (int x : casos[i]) arv.inserir(x);
        int r = 0;
        arv.chaveRaiz(r);
        checar(r == 20 && arv.altura() == 2 && valida(arv),
               nomes[i] + ": raiz 20, altura 2, " + to_string(arv.estatisticas().rotacoes) + " rotacao(oes)");
    }
    arv.limpar();
}

void testeOrdemInsercao(ArvoreBusca &arv, const string &tipo, int n) {
    cout << "\n> Insercao em ordem " << tipo << " (" << n << " elementos)\n";
    arv.limpar();
    arv.zerarEstatisticas();
    vector<int> seq = sequencia(n, tipo);
    checar(inserirVerificando(arv, seq), "todas as insercoes mantiveram as propriedades");
    vector<int> v = arv.emOrdem();
    checar((int)v.size() == n && ordenadoSemRepeticao(v), "percurso em ordem sai crescente e completo");
    cout << "          altura final = " << arv.altura()
         << ", rotacoes = " << arv.estatisticas().rotacoes << "\n";
    arv.limpar();
}

void testeBusca(ArvoreBusca &arv) {
    cout << "\n> Busca de elementos existentes e inexistentes\n";
    arv.limpar();
    for (int x : sequencia(100, "aleatoria")) arv.inserir(x * 10); // 10, 20, ..., 1000
    bool todos = true;
    for (int x = 10; x <= 1000; x += 10) todos = todos && arv.buscar(x);
    checar(todos, "os 100 IDs inseridos sao encontrados");
    bool nenhum = true;
    for (int x : {-5, 0, 5, 15, 555, 1001, 99999}) nenhum = nenhum && !arv.buscar(x);
    checar(nenhum, "IDs que nao existem (menores, maiores e no meio) nao sao encontrados");
    arv.limpar();
}

// Procura na arvore um no com a quantidade de filhos pedida, remove ele
// e confere se tudo continua certo.
void removerComFilhos(ArvoreBusca &arv, int qtdFilhos, const string &descricao) {
    int alvo = 0;
    bool achou = false;
    for (int x : arv.emOrdem()) {
        if (arv.filhos(x) == qtdFilhos) {
            alvo = x;
            achou = true;
            break;
        }
    }
    if (!achou) {
        checar(false, "nao achei no com " + to_string(qtdFilhos) + " filho(s) para testar");
        return;
    }
    int antes = arv.tamanho();
    bool ok = arv.remover(alvo);
    checar(ok && !arv.buscar(alvo) && arv.tamanho() == antes - 1 && valida(arv) &&
               ordenadoSemRepeticao(arv.emOrdem()),
           descricao + " (removido " + to_string(alvo) + ")");
}

void testeRemocao(ArvoreBusca &arv) {
    cout << "\n> Remocao com diferentes configuracoes de filhos\n";
    arv.limpar();
    for (int x : {50, 30, 70, 20, 40, 60, 80, 10, 35, 65, 90, 5}) arv.inserir(x);
    removerComFilhos(arv, 0, "remover folha");
    removerComFilhos(arv, 1, "remover no com 1 filho");
    removerComFilhos(arv, 2, "remover no com 2 filhos");

    int r = 0;
    arv.chaveRaiz(r);
    int antes = arv.tamanho();
    checar(arv.remover(r) && !arv.buscar(r) && arv.tamanho() == antes - 1 && valida(arv),
           "remover a raiz (" + to_string(r) + ")");

    cout << "\n> Remocao de elementos inexistentes\n";
    antes = arv.tamanho();
    vector<int> antesV = arv.emOrdem();
    checar(!arv.remover(1234) && !arv.remover(-1), "remover 1234 e -1 retorna false");
    checar(arv.tamanho() == antes && arv.emOrdem() == antesV && valida(arv), "arvore ficou igual");

    cout << "\n> Remover tudo ate esvaziar (ordem aleatoria)\n";
    vector<int> resto = arv.emOrdem();
    shuffle(resto.begin(), resto.end(), mt19937(7));
    bool tudoOk = true;
    for (int x : resto) tudoOk = tudoOk && arv.remover(x) && valida(arv);
    checar(tudoOk && arv.vazia() && arv.altura() == 0, "removeu todos e a arvore ficou vazia e valida");
    arv.limpar();
}

// Muitas operacoes aleatorias comparando com std::set (que serve de
// gabarito). Verifica as propriedades periodicamente.
void testeEstresse(ArvoreBusca &arv) {
    cout << "\n> Estresse: 20000 operacoes aleatorias comparadas com std::set\n";
    arv.limpar();
    set<int> gabarito;
    mt19937 rng(2024);
    uniform_int_distribution<int> valor(1, 3000), tipoOp(0, 2);
    bool iguais = true, sempreValida = true;
    for (int i = 1; i <= 20000; i++) {
        int x = valor(rng);
        int op = tipoOp(rng);
        if (op == 0) {
            iguais = iguais && (arv.inserir(x) == gabarito.insert(x).second);
        } else if (op == 1) {
            iguais = iguais && (arv.remover(x) == (gabarito.erase(x) == 1));
        } else {
            iguais = iguais && (arv.buscar(x) == (gabarito.count(x) == 1));
        }
        if (i % 250 == 0) sempreValida = sempreValida && valida(arv);
    }
    vector<int> esperado(gabarito.begin(), gabarito.end());
    checar(iguais, "todas as respostas iguais as do std::set");
    checar(sempreValida && valida(arv), "propriedades validas durante e no fim");
    checar(arv.emOrdem() == esperado, "conteudo final igual ao do std::set (" + to_string(esperado.size()) + " elementos)");
    arv.limpar();
}

void rodarTodos(ArvoreBusca &arv) {
    cout << "\n==================== " << arv.nome() << " ====================\n";
    testeArvoreVazia(arv);
    testeUmElemento(arv);
    testeRepetidos(arv);
    testeCasosRotacao(arv);
    testeOrdemInsercao(arv, "crescente", 1000);
    testeOrdemInsercao(arv, "decrescente", 1000);
    testeOrdemInsercao(arv, "aleatoria", 1000);
    testeBusca(arv);
    testeRemocao(arv);
    testeEstresse(arv);
}

int main() {
    AVL avl;
    RubroNegra rb;
    rodarTodos(avl);
    rodarTodos(rb);

    cout << "\n==============================================\n";
    cout << "Resultado: " << totalTestes - falhas << " de " << totalTestes << " testes passaram";
    if (falhas) cout << " (" << falhas << " falharam)";
    cout << "\n";
    return falhas ? 1 : 0;
}
