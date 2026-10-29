#ifndef RUBRONEGRA_H
#define RUBRONEGRA_H

#include "arvore.h"

enum Cor { VERMELHO, PRETO };

struct NoRB {
    int chave;
    Cor cor;
    NoRB *esq;
    NoRB *dir;
    NoRB *pai;
};

// Implementacao baseada no Cormen (Algoritmos, cap. 13), usando um no
// sentinela NIL (preto) no lugar de nullptr. Com o sentinela as folhas
// "existem" e sao pretas, o que simplifica bastante os casos do fixup.
class RubroNegra : public ArvoreBusca {
public:
    RubroNegra();
    ~RubroNegra();
    RubroNegra(const RubroNegra &) = delete;
    RubroNegra &operator=(const RubroNegra &) = delete;

    bool inserir(int chave) override;
    bool remover(int chave) override;
    bool buscar(int chave) override;

    std::vector<int> emOrdem() const override;
    void imprimir(std::ostream &out, bool usarCores) const override;

    int altura() const override;
    int tamanho() const override { return qtd; }
    int alturaNegra() const; // quantidade de nos pretos da raiz ate uma folha

    bool verificar(std::string &erro) const override;
    int filhos(int chave) const override;
    bool chaveRaiz(int &chave) const override;

    std::string nome() const override { return "Rubro-Negra"; }
    void limpar() override;

private:
    NoRB *NIL;
    NoRB *raiz;
    int qtd;

    void pintar(NoRB *no, Cor cor);
    void rotacaoEsquerda(NoRB *x);
    void rotacaoDireita(NoRB *y);
    void corrigirInsercao(NoRB *z);
    void corrigirRemocao(NoRB *x);
    void transplantar(NoRB *u, NoRB *v);
    NoRB *minimo(NoRB *no) const;
    NoRB *encontrar(int chave) const;

    int altura(NoRB *no) const;
    void emOrdem(NoRB *no, std::vector<int> &v) const;
    void imprimir(std::ostream &out, NoRB *no, const std::string &prefixo,
                  bool ultimo, const std::string &lado, bool usarCores) const;
    std::string rotulo(NoRB *no, bool usarCores) const;
    int verificar(NoRB *no, long long min, long long max, std::string &erro) const;
    void liberar(NoRB *no);
};

#endif
