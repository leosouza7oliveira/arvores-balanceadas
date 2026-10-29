#ifndef ARVORE_H
#define ARVORE_H

#include <ostream>
#include <string>
#include <vector>

// Contadores que cada arvore vai acumulando. Servem para a analise
// comparativa (quantas rotacoes, quantas trocas de cor etc).
struct Estatisticas {
    long long rotacoes = 0;       // toda rotacao simples conta 1 (uma dupla conta 2)
    long long rotacoesDuplas = 0; // so a AVL usa
    long long recoloracoes = 0;   // so a Rubro-Negra usa (nos que mudaram de cor)
    long long comparacoes = 0;    // comparacoes de chave feitas nas buscas
};

// Classe base com as operacoes que as duas arvores precisam ter.
// O main trabalha com um ponteiro para ArvoreBusca, assim o usuario
// escolhe a arvore na hora da execucao e o resto do codigo e o mesmo.
class ArvoreBusca {
public:
    virtual ~ArvoreBusca() {}

    virtual bool inserir(int chave) = 0;  // retorna false se a chave ja existe
    virtual bool remover(int chave) = 0;  // retorna false se a chave nao existe
    virtual bool buscar(int chave) = 0;   // nao e const porque conta comparacoes

    virtual std::vector<int> emOrdem() const = 0;
    virtual void imprimir(std::ostream &out, bool usarCores) const = 0;

    // Altura medida em niveis: arvore vazia = 0, so a raiz = 1
    virtual int altura() const = 0;
    virtual int tamanho() const = 0;
    virtual bool vazia() const { return tamanho() == 0; }

    // Confere todas as propriedades da estrutura (ordem de busca e
    // balanceamento). Se algo estiver errado, explica em "erro".
    virtual bool verificar(std::string &erro) const = 0;

    // Usados nos testes de remocao: -1 se a chave nao existe
    virtual int filhos(int chave) const = 0;
    virtual bool chaveRaiz(int &chave) const = 0;

    virtual std::string nome() const = 0;
    virtual void limpar() = 0;

    const Estatisticas &estatisticas() const { return stats; }
    void zerarEstatisticas() { stats = Estatisticas(); }

protected:
    Estatisticas stats;
};

#endif
