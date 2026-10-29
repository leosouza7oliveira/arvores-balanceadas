#include "rubronegra.h"

#include <algorithm>
#include <sstream>

RubroNegra::RubroNegra() : qtd(0) {
    NIL = new NoRB{0, PRETO, nullptr, nullptr, nullptr};
    NIL->esq = NIL->dir = NIL->pai = NIL;
    raiz = NIL;
}

RubroNegra::~RubroNegra() {
    liberar(raiz);
    delete NIL;
}

void RubroNegra::liberar(NoRB *no) {
    if (no == NIL) return;
    liberar(no->esq);
    liberar(no->dir);
    delete no;
}

void RubroNegra::limpar() {
    liberar(raiz);
    raiz = NIL;
    qtd = 0;
}

// Troca a cor e conta como recoloracao so se a cor realmente mudou.
// O NIL e sempre preto, entao nunca mexe nele.
void RubroNegra::pintar(NoRB *no, Cor cor) {
    if (no == NIL || no->cor == cor) return;
    no->cor = cor;
    stats.recoloracoes++;
}

// ---------------------------------------------------------------
// Rotacoes (mesma ideia da AVL, mas aqui tem que acertar o ponteiro
// para o pai tambem)
// ---------------------------------------------------------------
void RubroNegra::rotacaoEsquerda(NoRB *x) {
    NoRB *y = x->dir;
    x->dir = y->esq;
    if (y->esq != NIL) y->esq->pai = x;
    y->pai = x->pai;
    if (x->pai == NIL)
        raiz = y;
    else if (x == x->pai->esq)
        x->pai->esq = y;
    else
        x->pai->dir = y;
    y->esq = x;
    x->pai = y;
    stats.rotacoes++;
}

void RubroNegra::rotacaoDireita(NoRB *y) {
    NoRB *x = y->esq;
    y->esq = x->dir;
    if (x->dir != NIL) x->dir->pai = y;
    x->pai = y->pai;
    if (y->pai == NIL)
        raiz = x;
    else if (y == y->pai->dir)
        y->pai->dir = x;
    else
        y->pai->esq = x;
    x->dir = y;
    y->pai = x;
    stats.rotacoes++;
}

// ---------------------------------------------------------------
// Insercao: insere como numa ABB comum, pinta o no de vermelho e
// depois corrige as cores subindo pela arvore.
// ---------------------------------------------------------------
bool RubroNegra::inserir(int chave) {
    NoRB *pai = NIL;
    NoRB *atual = raiz;
    while (atual != NIL) {
        pai = atual;
        if (chave < atual->chave)
            atual = atual->esq;
        else if (chave > atual->chave)
            atual = atual->dir;
        else
            return false; // repetido, nao insere
    }

    NoRB *z = new NoRB{chave, VERMELHO, NIL, NIL, pai};
    if (pai == NIL)
        raiz = z;
    else if (chave < pai->chave)
        pai->esq = z;
    else
        pai->dir = z;

    qtd++;
    corrigirInsercao(z);
    return true;
}

// O unico problema possivel depois de inserir e ter vermelho com pai
// vermelho (ou a raiz vermelha). Casos, com o pai sendo filho esquerdo
// do avo (o outro lado e espelhado):
//   caso 1: tio vermelho  -> pai e tio viram pretos, avo vira vermelho,
//           e o problema sobe para o avo
//   caso 2: tio preto e z e filho direito -> rotaciona o pai para cair
//           no caso 3
//   caso 3: tio preto e z e filho esquerdo -> pai vira preto, avo vira
//           vermelho e rotaciona o avo para a direita
void RubroNegra::corrigirInsercao(NoRB *z) {
    while (z->pai->cor == VERMELHO) {
        NoRB *avo = z->pai->pai;
        if (z->pai == avo->esq) {
            NoRB *tio = avo->dir;
            if (tio->cor == VERMELHO) {
                pintar(z->pai, PRETO);
                pintar(tio, PRETO);
                pintar(avo, VERMELHO);
                z = avo;
            } else {
                if (z == z->pai->dir) {
                    z = z->pai;
                    rotacaoEsquerda(z);
                }
                pintar(z->pai, PRETO);
                pintar(z->pai->pai, VERMELHO);
                rotacaoDireita(z->pai->pai);
            }
        } else {
            NoRB *tio = avo->esq;
            if (tio->cor == VERMELHO) {
                pintar(z->pai, PRETO);
                pintar(tio, PRETO);
                pintar(avo, VERMELHO);
                z = avo;
            } else {
                if (z == z->pai->esq) {
                    z = z->pai;
                    rotacaoDireita(z);
                }
                pintar(z->pai, PRETO);
                pintar(z->pai->pai, VERMELHO);
                rotacaoEsquerda(z->pai->pai);
            }
        }
    }
    pintar(raiz, PRETO);
}

// ---------------------------------------------------------------
// Remocao
// ---------------------------------------------------------------

// Coloca a subarvore v no lugar da subarvore u
void RubroNegra::transplantar(NoRB *u, NoRB *v) {
    if (u->pai == NIL)
        raiz = v;
    else if (u == u->pai->esq)
        u->pai->esq = v;
    else
        u->pai->dir = v;
    v->pai = u->pai; // pode ser o NIL, o fixup usa isso
}

NoRB *RubroNegra::minimo(NoRB *no) const {
    while (no->esq != NIL) no = no->esq;
    return no;
}

NoRB *RubroNegra::encontrar(int chave) const {
    NoRB *atual = raiz;
    while (atual != NIL && atual->chave != chave)
        atual = chave < atual->chave ? atual->esq : atual->dir;
    return atual;
}

bool RubroNegra::remover(int chave) {
    NoRB *z = encontrar(chave);
    if (z == NIL) return false;

    NoRB *y = z;          // no que de fato sai da posicao dele
    Cor corOriginal = y->cor;
    NoRB *x;              // no que ocupa o lugar de y

    if (z->esq == NIL) {
        x = z->dir;
        transplantar(z, z->dir);
    } else if (z->dir == NIL) {
        x = z->esq;
        transplantar(z, z->esq);
    } else {
        // dois filhos: usa o sucessor (igual na AVL)
        y = minimo(z->dir);
        corOriginal = y->cor;
        x = y->dir;
        if (y->pai == z) {
            x->pai = y;
        } else {
            transplantar(y, y->dir);
            y->dir = z->dir;
            y->dir->pai = y;
        }
        transplantar(z, y);
        y->esq = z->esq;
        y->esq->pai = y;
        pintar(y, z->cor); // o sucessor herda a cor do no removido
    }
    delete z;
    qtd--;

    // Se saiu um no preto, algum caminho ficou com um preto a menos
    if (corOriginal == PRETO) corrigirRemocao(x);
    return true;
}

// x carrega um "preto extra". Casos com x filho esquerdo (o outro lado e
// espelhado), sendo w o irmao de x:
//   caso 1: w vermelho -> troca cores de w e pai, rotaciona o pai, e cai
//           em um dos outros casos
//   caso 2: w preto com dois filhos pretos -> w vira vermelho e o preto
//           extra sobe para o pai
//   caso 3: w preto, filho esq de w vermelho e dir preto -> rotaciona w
//           para cair no caso 4
//   caso 4: w preto com filho dir vermelho -> rotaciona o pai e acaba
void RubroNegra::corrigirRemocao(NoRB *x) {
    while (x != raiz && x->cor == PRETO) {
        if (x == x->pai->esq) {
            NoRB *w = x->pai->dir;
            if (w->cor == VERMELHO) {
                pintar(w, PRETO);
                pintar(x->pai, VERMELHO);
                rotacaoEsquerda(x->pai);
                w = x->pai->dir;
            }
            if (w->esq->cor == PRETO && w->dir->cor == PRETO) {
                pintar(w, VERMELHO);
                x = x->pai;
            } else {
                if (w->dir->cor == PRETO) {
                    pintar(w->esq, PRETO);
                    pintar(w, VERMELHO);
                    rotacaoDireita(w);
                    w = x->pai->dir;
                }
                pintar(w, x->pai->cor);
                pintar(x->pai, PRETO);
                pintar(w->dir, PRETO);
                rotacaoEsquerda(x->pai);
                x = raiz;
            }
        } else {
            NoRB *w = x->pai->esq;
            if (w->cor == VERMELHO) {
                pintar(w, PRETO);
                pintar(x->pai, VERMELHO);
                rotacaoDireita(x->pai);
                w = x->pai->esq;
            }
            if (w->dir->cor == PRETO && w->esq->cor == PRETO) {
                pintar(w, VERMELHO);
                x = x->pai;
            } else {
                if (w->esq->cor == PRETO) {
                    pintar(w->dir, PRETO);
                    pintar(w, VERMELHO);
                    rotacaoEsquerda(w);
                    w = x->pai->esq;
                }
                pintar(w, x->pai->cor);
                pintar(x->pai, PRETO);
                pintar(w->esq, PRETO);
                rotacaoDireita(x->pai);
                x = raiz;
            }
        }
    }
    pintar(x, PRETO);
}

// ---------------------------------------------------------------
// Busca, percurso e funcoes auxiliares
// ---------------------------------------------------------------
bool RubroNegra::buscar(int chave) {
    NoRB *atual = raiz;
    while (atual != NIL) {
        stats.comparacoes++;
        if (chave == atual->chave) return true;
        atual = chave < atual->chave ? atual->esq : atual->dir;
    }
    return false;
}

std::vector<int> RubroNegra::emOrdem() const {
    std::vector<int> v;
    v.reserve(qtd);
    emOrdem(raiz, v);
    return v;
}

void RubroNegra::emOrdem(NoRB *no, std::vector<int> &v) const {
    if (no == NIL) return;
    emOrdem(no->esq, v);
    v.push_back(no->chave);
    emOrdem(no->dir, v);
}

int RubroNegra::altura() const { return altura(raiz); }

int RubroNegra::altura(NoRB *no) const {
    if (no == NIL) return 0;
    return 1 + std::max(altura(no->esq), altura(no->dir));
}

int RubroNegra::alturaNegra() const {
    int bh = 0;
    for (NoRB *no = raiz; no != NIL; no = no->esq)
        if (no->cor == PRETO) bh++;
    return bh;
}

bool RubroNegra::chaveRaiz(int &chave) const {
    if (raiz == NIL) return false;
    chave = raiz->chave;
    return true;
}

int RubroNegra::filhos(int chave) const {
    NoRB *no = encontrar(chave);
    if (no == NIL) return -1;
    return (no->esq != NIL) + (no->dir != NIL);
}

// ---------------------------------------------------------------
// Impressao: cada no aparece com [V] (vermelho) ou [P] (preto).
// Se usarCores for true, os vermelhos tambem saem em vermelho no
// terminal (codigo ANSI).
// ---------------------------------------------------------------
std::string RubroNegra::rotulo(NoRB *no, bool usarCores) const {
    std::ostringstream ss;
    if (no->cor == VERMELHO) {
        if (usarCores) ss << "\033[1;31m";
        ss << no->chave << " [V]";
        if (usarCores) ss << "\033[0m";
    } else {
        ss << no->chave << " [P]";
    }
    return ss.str();
}

void RubroNegra::imprimir(std::ostream &out, bool usarCores) const {
    if (raiz == NIL) {
        out << "(arvore vazia)\n";
        return;
    }
    out << rotulo(raiz, usarCores) << "\n";
    if (raiz->esq != NIL || raiz->dir != NIL) {
        imprimir(out, raiz->esq, "", false, "E", usarCores);
        imprimir(out, raiz->dir, "", true, "D", usarCores);
    }
}

void RubroNegra::imprimir(std::ostream &out, NoRB *no, const std::string &prefixo,
                          bool ultimo, const std::string &lado, bool usarCores) const {
    out << prefixo << (ultimo ? "`-- " : "|-- ") << lado << ": ";
    if (no == NIL) {
        out << "nulo\n";
        return;
    }
    out << rotulo(no, usarCores) << "\n";
    if (no->esq != NIL || no->dir != NIL) {
        std::string novo = prefixo + (ultimo ? "    " : "|   ");
        imprimir(out, no->esq, novo, false, "E", usarCores);
        imprimir(out, no->dir, novo, true, "D", usarCores);
    }
}

// ---------------------------------------------------------------
// Verificacao das 5 propriedades:
//  1. todo no e vermelho ou preto (garantido pelo enum)
//  2. a raiz e preta
//  3. as folhas (NIL) sao pretas
//  4. no vermelho nao tem filho vermelho
//  5. todo caminho de um no ate as folhas tem a mesma qtd de pretos
// Alem disso confere a ordem da ABB e os ponteiros de pai.
// Retorna a altura negra da subarvore, ou -1 se achou erro.
// ---------------------------------------------------------------
bool RubroNegra::verificar(std::string &erro) const {
    erro.clear();
    if (NIL->cor != PRETO) {
        erro = "sentinela NIL nao esta preto";
        return false;
    }
    if (raiz != NIL && raiz->cor != PRETO) {
        erro = "raiz nao e preta";
        return false;
    }
    if (raiz != NIL && raiz->pai != NIL) {
        erro = "pai da raiz deveria ser NIL";
        return false;
    }
    if (verificar(raiz, -(1LL << 40), (1LL << 40), erro) < 0) return false;
    if ((int)emOrdem().size() != qtd) {
        erro = "contador de elementos nao bate com a arvore";
        return false;
    }
    return true;
}

int RubroNegra::verificar(NoRB *no, long long min, long long max, std::string &erro) const {
    if (no == NIL) return 1; // NIL conta como preto
    std::ostringstream ss;

    if (no->chave <= min || no->chave >= max) {
        ss << "no " << no->chave << " quebra a ordem da arvore de busca";
        erro = ss.str();
        return -1;
    }
    if ((no->esq != NIL && no->esq->pai != no) || (no->dir != NIL && no->dir->pai != no)) {
        ss << "ponteiro de pai errado embaixo do no " << no->chave;
        erro = ss.str();
        return -1;
    }
    if (no->cor == VERMELHO && (no->esq->cor == VERMELHO || no->dir->cor == VERMELHO)) {
        ss << "no vermelho " << no->chave << " tem filho vermelho";
        erro = ss.str();
        return -1;
    }

    int be = verificar(no->esq, min, no->chave, erro);
    if (be < 0) return -1;
    int bd = verificar(no->dir, no->chave, max, erro);
    if (bd < 0) return -1;
    if (be != bd) {
        ss << "altura negra diferente nos dois lados do no " << no->chave;
        erro = ss.str();
        return -1;
    }
    return be + (no->cor == PRETO ? 1 : 0);
}
