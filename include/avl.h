#ifndef AVL_H
#define AVL_H

#include "arvore.h"

struct NoAVL {
    int chave;
    int altura; // altura da subarvore com raiz neste no (folha = 1)
    NoAVL *esq;
    NoAVL *dir;

    NoAVL(int c) : chave(c), altura(1), esq(nullptr), dir(nullptr) {}
};

class AVL : public ArvoreBusca {
public:
    AVL();
    ~AVL();
    AVL(const AVL &) = delete;            // nao precisamos copiar arvore,
    AVL &operator=(const AVL &) = delete; // entao bloqueia para evitar double free

    bool inserir(int chave) override;
    bool remover(int chave) override;
    bool buscar(int chave) override;

    std::vector<int> emOrdem() const override;
    void imprimir(std::ostream &out, bool usarCores) const override;

    int altura() const override;
    int tamanho() const override { return qtd; }

    bool verificar(std::string &erro) const override;
    int filhos(int chave) const override;
    bool chaveRaiz(int &chave) const override;

    std::string nome() const override { return "AVL"; }
    void limpar() override;

private:
    NoAVL *raiz;
    int qtd;

    static int h(NoAVL *no) { return no ? no->altura : 0; }
    static int fatorBalanceamento(NoAVL *no);
    static void atualizarAltura(NoAVL *no);

    NoAVL *rotacaoDireita(NoAVL *y);
    NoAVL *rotacaoEsquerda(NoAVL *x);
    NoAVL *balancear(NoAVL *no);

    NoAVL *inserir(NoAVL *no, int chave, bool &inseriu);
    NoAVL *remover(NoAVL *no, int chave, bool &removeu);
    NoAVL *removerMinimo(NoAVL *no, NoAVL *&minimo);

    void emOrdem(NoAVL *no, std::vector<int> &v) const;
    void imprimir(std::ostream &out, NoAVL *no, const std::string &prefixo,
                  bool ultimo, const std::string &lado) const;
    int verificar(NoAVL *no, long long min, long long max, std::string &erro) const;
    void liberar(NoAVL *no);
};

#endif
