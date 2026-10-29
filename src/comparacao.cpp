// =====================================================================
// Analise comparativa AVL x Rubro-Negra
//
// Usa os mesmos conjuntos de dados e a mesma sequencia de operacoes nas
// duas arvores e mede: altura, rotacoes, recoloracoes, comparacoes nas
// buscas e tempo. Tambem roda uma ABB comum (sem balanceamento) so para
// mostrar o que acontece quando os IDs chegam ordenados.
//
// Experimento 1: insere n IDs (crescente, decrescente, aleatoria),
//                busca todos, depois remove metade.
// Experimento 2: simulacao da Black Friday (IDs novos sempre crescentes,
//                misturando cadastro, rastreio e entrega).
//
// Os resultados saem na tela e em resultados/*.csv
// Executar: ./bin/comparacao
// =====================================================================

#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "avl.h"
#include "rubronegra.h"

using namespace std;
using namespace std::chrono;

const int REPETICOES = 3; // o tempo e a media de 3 execucoes

// ---------------------------------------------------------------------
// ABB comum, sem balanceamento. Feita de forma iterativa porque com
// IDs crescentes ela vira uma lista e a recursao estouraria a pilha.
// ---------------------------------------------------------------------
struct ABB {
    struct No {
        int chave;
        No *esq = nullptr, *dir = nullptr;
        No(int c) : chave(c) {}
    };
    No *raiz = nullptr;

    ~ABB() {
        vector<No *> pilha;
        if (raiz) pilha.push_back(raiz);
        while (!pilha.empty()) {
            No *no = pilha.back();
            pilha.pop_back();
            if (no->esq) pilha.push_back(no->esq);
            if (no->dir) pilha.push_back(no->dir);
            delete no;
        }
    }

    void inserir(int chave) {
        No **atual = &raiz;
        while (*atual) {
            if (chave == (*atual)->chave) return;
            atual = chave < (*atual)->chave ? &(*atual)->esq : &(*atual)->dir;
        }
        *atual = new No(chave);
    }

    long long buscar(int chave) { // devolve quantas comparacoes fez
        long long c = 0;
        No *atual = raiz;
        while (atual) {
            c++;
            if (chave == atual->chave) break;
            atual = chave < atual->chave ? atual->esq : atual->dir;
        }
        return c;
    }

    int altura() {
        int h = 0;
        vector<pair<No *, int>> pilha;
        if (raiz) pilha.push_back({raiz, 1});
        while (!pilha.empty()) {
            auto [no, nivel] = pilha.back();
            pilha.pop_back();
            h = max(h, nivel);
            if (no->esq) pilha.push_back({no->esq, nivel + 1});
            if (no->dir) pilha.push_back({no->dir, nivel + 1});
        }
        return h;
    }
};

vector<int> gerar(int n, const string &ordem, unsigned semente) {
    vector<int> v(n);
    iota(v.begin(), v.end(), 1);
    if (ordem == "decrescente") reverse(v.begin(), v.end());
    if (ordem == "aleatoria") shuffle(v.begin(), v.end(), mt19937(semente));
    return v;
}

double msDesde(steady_clock::time_point inicio) {
    return duration<double, milli>(steady_clock::now() - inicio).count();
}

struct Resultado {
    int altura = 0, alturaAposRemocao = 0;
    long long rotInsercao = 0, rotRemocao = 0;
    long long corInsercao = 0, corRemocao = 0;
    double comparacoesMedias = 0;
    double tInsercao = 0, tBusca = 0, tRemocao = 0;
};

// Roda a sequencia completa em uma arvore. Cria uma arvore nova a cada
// repeticao para nao aproveitar nada da anterior.
template <class Arvore>
Resultado executar(const vector<int> &dados, const vector<int> &remocoes) {
    Resultado r;
    for (int rep = 0; rep < REPETICOES; rep++) {
        Arvore arv;

        auto t0 = steady_clock::now();
        for (int x : dados) arv.inserir(x);
        r.tInsercao += msDesde(t0);
        Estatisticas aposIns = arv.estatisticas();

        t0 = steady_clock::now();
        for (int x : dados) arv.buscar(x);
        r.tBusca += msDesde(t0);
        long long comps = arv.estatisticas().comparacoes - aposIns.comparacoes;

        t0 = steady_clock::now();
        for (int x : remocoes) arv.remover(x);
        r.tRemocao += msDesde(t0);

        if (rep == 0) { // os contadores sao iguais em toda repeticao
            r.rotInsercao = aposIns.rotacoes;
            r.corInsercao = aposIns.recoloracoes;
            r.rotRemocao = arv.estatisticas().rotacoes - aposIns.rotacoes;
            r.corRemocao = arv.estatisticas().recoloracoes - aposIns.recoloracoes;
            r.comparacoesMedias = (double)comps / dados.size();
            r.alturaAposRemocao = arv.altura();
        }
    }
    r.tInsercao /= REPETICOES;
    r.tBusca /= REPETICOES;
    r.tRemocao /= REPETICOES;
    return r;
}

// A altura logo depois das insercoes precisa de uma arvore separada,
// porque executar() ja removeu metade.
template <class Arvore>
int alturaAposInserir(const vector<int> &dados) {
    Arvore arv;
    for (int x : dados) arv.inserir(x);
    return arv.altura();
}

void experimentoInsercaoRemocao() {
    cout << "\n=== Experimento 1: insercao, busca e remocao de metade ===\n";
    cout << "(tempos em ms, media de " << REPETICOES << " execucoes)\n\n";

    ofstream csv("resultados/insercao_remocao.csv");
    csv << "n,ordem,estrutura,altura,altura_minima,rotacoes_insercao,recoloracoes_insercao,"
           "comparacoes_medias_busca,altura_apos_remover_metade,rotacoes_remocao,"
           "recoloracoes_remocao,tempo_insercao_ms,tempo_busca_ms,tempo_remocao_ms\n";

    cout << left << setw(9) << "n" << setw(13) << "ordem" << setw(13) << "estrutura" << right
         << setw(7) << "alt" << setw(7) << "min" << setw(10) << "rot.ins" << setw(10) << "cor.ins"
         << setw(8) << "comp" << setw(8) << "alt/2" << setw(10) << "rot.rem" << setw(10)
         << "cor.rem" << setw(10) << "t.ins" << setw(10) << "t.busca" << setw(10) << "t.rem" << "\n";

    for (int n : {1000, 10000, 100000, 1000000}) {
        int minima = (int)ceil(log2(n + 1.0));
        for (string ordem : {"crescente", "decrescente", "aleatoria"}) {
            vector<int> dados = gerar(n, ordem, 42);
            // remove metade dos IDs, escolhidos em ordem aleatoria (mesma lista nas duas)
            vector<int> remocoes = gerar(n, "aleatoria", 7);
            remocoes.resize(n / 2);

            Resultado avl = executar<AVL>(dados, remocoes);
            Resultado rb = executar<RubroNegra>(dados, remocoes);
            avl.altura = alturaAposInserir<AVL>(dados);
            rb.altura = alturaAposInserir<RubroNegra>(dados);

            auto linha = [&](const string &nome, const Resultado &r) {
                cout << left << setw(9) << n << setw(13) << ordem << setw(13) << nome << right
                     << setw(7) << r.altura << setw(7) << minima << setw(10) << r.rotInsercao
                     << setw(10) << r.corInsercao << setw(8) << fixed << setprecision(2)
                     << r.comparacoesMedias << setw(8) << r.alturaAposRemocao << setw(10)
                     << r.rotRemocao << setw(10) << r.corRemocao << setw(10) << setprecision(2)
                     << r.tInsercao << setw(10) << r.tBusca << setw(10) << r.tRemocao << "\n";
                csv << n << "," << ordem << "," << nome << "," << r.altura << "," << minima << ","
                    << r.rotInsercao << "," << r.corInsercao << "," << fixed << setprecision(3)
                    << r.comparacoesMedias << "," << r.alturaAposRemocao << "," << r.rotRemocao
                    << "," << r.corRemocao << "," << r.tInsercao << "," << r.tBusca << ","
                    << r.tRemocao << "\n";
            };
            linha("AVL", avl);
            linha("Rubro-Negra", rb);

            // ABB comum: so insercao e busca. Com dados ordenados e n grande
            // ela levaria minutos (O(n^2)), entao pulo esses casos.
            if (ordem == "aleatoria" || n <= 10000) {
                ABB abb;
                auto t0 = steady_clock::now();
                for (int x : dados) abb.inserir(x);
                double tIns = msDesde(t0);
                long long comps = 0;
                t0 = steady_clock::now();
                for (int x : dados) comps += abb.buscar(x);
                double tBusca = msDesde(t0);
                double media = (double)comps / n;
                cout << left << setw(9) << n << setw(13) << ordem << setw(13) << "ABB comum" << right
                     << setw(7) << abb.altura() << setw(7) << minima << setw(10) << "-" << setw(10)
                     << "-" << setw(8) << setprecision(2) << media << setw(8) << "-" << setw(10)
                     << "-" << setw(10) << "-" << setw(10) << tIns << setw(10) << tBusca << setw(10)
                     << "-" << "\n";
                csv << n << "," << ordem << ",ABB comum," << abb.altura() << "," << minima
                    << ",,," << fixed << setprecision(3) << media << ",,,," << tIns << "," << tBusca << ",\n";
            }
            cout << "\n";
        }
    }
}

// ---------------------------------------------------------------------
// Simulacao da Black Friday
//   - 50% das operacoes: pedido novo (ID = ultimo ID + 1, sempre crescente)
//   - 35%: cliente rastreando uma encomenda que existe (busca)
//   - 15%: encomenda entregue (remocao de um ID qualquer que esta ativo)
// A sequencia de operacoes e gerada uma vez e usada nas duas arvores.
// ---------------------------------------------------------------------
struct Operacao {
    char tipo; // 'i', 'b' ou 'r'
    int id;
};

vector<Operacao> gerarBlackFriday(int total, unsigned semente) {
    mt19937 rng(semente);
    uniform_int_distribution<int> sorteio(1, 100);
    vector<Operacao> ops;
    ops.reserve(total);
    vector<int> ativos; // IDs que estao no sistema agora
    int proximoID = 1000000;
    for (int i = 0; i < total; i++) {
        int s = sorteio(rng);
        if (ativos.empty() || s <= 50) {
            ops.push_back({'i', proximoID});
            ativos.push_back(proximoID++);
        } else {
            size_t pos = uniform_int_distribution<size_t>(0, ativos.size() - 1)(rng);
            if (s <= 85) {
                ops.push_back({'b', ativos[pos]});
            } else {
                ops.push_back({'r', ativos[pos]});
                ativos[pos] = ativos.back();
                ativos.pop_back();
            }
        }
    }
    return ops;
}

template <class Arvore>
void simular(const string &nome, const vector<Operacao> &ops, ofstream &csv) {
    double tempo = 0;
    long long ins = 0, bus = 0, rem = 0;
    for (const Operacao &op : ops) {
        if (op.tipo == 'i') ins++;
        else if (op.tipo == 'b') bus++;
        else rem++;
    }

    int alturaFinal = 0, tamanhoFinal = 0;
    Estatisticas e;
    for (int rep = 0; rep < REPETICOES; rep++) {
        Arvore arv;
        auto t0 = steady_clock::now();
        for (const Operacao &op : ops) {
            if (op.tipo == 'i') arv.inserir(op.id);
            else if (op.tipo == 'b') arv.buscar(op.id);
            else arv.remover(op.id);
        }
        tempo += msDesde(t0);
        alturaFinal = arv.altura();
        tamanhoFinal = arv.tamanho();
        e = arv.estatisticas();
    }
    tempo /= REPETICOES;

    int minima = (int)ceil(log2(tamanhoFinal + 1.0));
    cout << left << setw(13) << nome << right << setw(10) << tamanhoFinal << setw(8) << alturaFinal
         << setw(6) << minima << setw(12) << e.rotacoes << setw(13) << e.recoloracoes << setw(10)
         << fixed << setprecision(2) << (double)e.comparacoes / bus << setw(12) << tempo << "\n";
    csv << nome << "," << ops.size() << "," << ins << "," << bus << "," << rem << "," << tamanhoFinal
        << "," << alturaFinal << "," << minima << "," << e.rotacoes << "," << e.rotacoesDuplas << ","
        << e.recoloracoes << "," << fixed << setprecision(3) << (double)e.comparacoes / bus << "," << tempo
        << "\n";
}

void experimentoBlackFriday() {
    const int TOTAL = 1000000;
    cout << "\n=== Experimento 2: simulacao Black Friday (" << TOTAL << " operacoes) ===\n";
    cout << "50% pedidos novos (IDs crescentes), 35% rastreios, 15% entregas\n\n";

    vector<Operacao> ops = gerarBlackFriday(TOTAL, 2025);
    ofstream csv("resultados/black_friday.csv");
    csv << "estrutura,operacoes,insercoes,buscas,remocoes,elementos_final,altura_final,"
           "altura_minima,rotacoes,rotacoes_duplas,recoloracoes,comparacoes_medias_busca,tempo_ms\n";

    cout << left << setw(13) << "estrutura" << right << setw(10) << "n final" << setw(8) << "alt"
         << setw(6) << "min" << setw(12) << "rotacoes" << setw(13) << "recoloracoes" << setw(10)
         << "comp" << setw(12) << "tempo(ms)" << "\n";
    simular<AVL>("AVL", ops, csv);
    simular<RubroNegra>("Rubro-Negra", ops, csv);
}

int main() {
    experimentoInsercaoRemocao();
    experimentoBlackFriday();
    cout << "\nCSVs salvos em resultados/\n";
    return 0;
}
