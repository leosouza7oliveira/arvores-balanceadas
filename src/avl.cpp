#include "avl.h"

#include <algorithm>
#include <sstream>

AVL::AVL() : raiz(nullptr), qtd(0) {}

AVL::~AVL() { liberar(raiz); }

void AVL::liberar(NoAVL *no) {
    if (!no) return;
    liberar(no->esq);
    liberar(no->dir);
    delete no;
}

void AVL::limpar() {
    liberar(raiz);
    raiz = nullptr;
    qtd = 0;
}

// fb = altura(esq) - altura(dir). Na AVL ele precisa ficar entre -1 e 1.
int AVL::fatorBalanceamento(NoAVL *no) {
    return no ? h(no->esq) - h(no->dir) : 0;
}

void AVL::atualizarAltura(NoAVL *no) {
    no->altura = 1 + std::max(h(no->esq), h(no->dir));
}

/* ---------------------------------------------------------------
   Rotacoes

    rotacaoDireita(y):          rotacaoEsquerda(x):

          y            x            x                y
         / \          / \          / \              / \
        x   C  -->   A   y        A   y    -->     x   C
       / \              / \          / \          / \
      A   B            B   C        B   C        A   B
   --------------------------------------------------------------- */
NoAVL *AVL::rotacaoDireita(NoAVL *y) {
    NoAVL *x = y->esq;
    y->esq = x->dir;
    x->dir = y;
    atualizarAltura(y); // y desceu, entao atualiza ele primeiro
    atualizarAltura(x);
    stats.rotacoes++;
    return x;
}

NoAVL *AVL::rotacaoEsquerda(NoAVL *x) {
    NoAVL *y = x->dir;
    x->dir = y->esq;
    y->esq = x;
    atualizarAltura(x);
    atualizarAltura(y);
    stats.rotacoes++;
    return y;
}

// Chamado na volta da recursao, em cada no do caminho ate a raiz.
// Os 4 casos classicos:
//   fb > 1  e filho esq com fb >= 0  -> caso LL, rotacao simples a direita
//   fb > 1  e filho esq com fb <  0  -> caso LR, rotacao dupla (esq no filho, dir no no)
//   fb < -1 e filho dir com fb <= 0  -> caso RR, rotacao simples a esquerda
//   fb < -1 e filho dir com fb >  0  -> caso RL, rotacao dupla (dir no filho, esq no no)
// O caso do filho com fb = 0 so acontece na remocao, e ai a simples resolve.
NoAVL *AVL::balancear(NoAVL *no) {
    atualizarAltura(no);
    int fb = fatorBalanceamento(no);

    if (fb > 1) {
        if (fatorBalanceamento(no->esq) < 0) {
            no->esq = rotacaoEsquerda(no->esq);
            stats.rotacoesDuplas++;
        }
        return rotacaoDireita(no);
    }
    if (fb < -1) {
        if (fatorBalanceamento(no->dir) > 0) {
            no->dir = rotacaoDireita(no->dir);
            stats.rotacoesDuplas++;
        }
        return rotacaoEsquerda(no);
    }
    return no;
}

// ---------------------------------------------------------------
// Insercao
// ---------------------------------------------------------------
bool AVL::inserir(int chave) {
    bool inseriu = false;
    raiz = inserir(raiz, chave, inseriu);
    if (inseriu) qtd++;
    return inseriu;
}

NoAVL *AVL::inserir(NoAVL *no, int chave, bool &inseriu) {
    if (!no) {
        inseriu = true;
        return new NoAVL(chave);
    }
    if (chave < no->chave)
        no->esq = inserir(no->esq, chave, inseriu);
    else if (chave > no->chave)
        no->dir = inserir(no->dir, chave, inseriu);
    else
        return no; // repetido: nao insere e a arvore nao muda

    return balancear(no);
}

// ---------------------------------------------------------------
// Remocao
// Se o no tem dois filhos, ele e substituido pelo sucessor (menor
// elemento da subarvore direita). Escolhi o sucessor nas duas arvores
// para a comparacao ficar justa.
// ---------------------------------------------------------------
bool AVL::remover(int chave) {
    bool removeu = false;
    raiz = remover(raiz, chave, removeu);
    if (removeu) qtd--;
    return removeu;
}

NoAVL *AVL::remover(NoAVL *no, int chave, bool &removeu) {
    if (!no) return nullptr; // chegou no fim e nao achou

    if (chave < no->chave) {
        no->esq = remover(no->esq, chave, removeu);
    } else if (chave > no->chave) {
        no->dir = remover(no->dir, chave, removeu);
    } else {
        removeu = true;
        if (!no->esq || !no->dir) {
            // 0 ou 1 filho: o filho (ou nullptr) sobe no lugar
            NoAVL *filho = no->esq ? no->esq : no->dir;
            delete no;
            return filho;
        }
        // 2 filhos: tira o sucessor da subarvore direita e coloca no lugar
        NoAVL *suc = nullptr;
        NoAVL *novaDir = removerMinimo(no->dir, suc);
        suc->esq = no->esq;
        suc->dir = novaDir;
        delete no;
        no = suc;
    }
    return balancear(no);
}

// Desliga o menor no da subarvore e devolve ele em "minimo",
// rebalanceando o caminho na volta.
NoAVL *AVL::removerMinimo(NoAVL *no, NoAVL *&minimo) {
    if (!no->esq) {
        minimo = no;
        return no->dir;
    }
    no->esq = removerMinimo(no->esq, minimo);
    return balancear(no);
}

// ---------------------------------------------------------------
// Busca (iterativa, conta as comparacoes)
// ---------------------------------------------------------------
bool AVL::buscar(int chave) {
    NoAVL *atual = raiz;
    while (atual) {
        stats.comparacoes++;
        if (chave == atual->chave) return true;
        atual = chave < atual->chave ? atual->esq : atual->dir;
    }
    return false;
}

std::vector<int> AVL::emOrdem() const {
    std::vector<int> v;
    v.reserve(qtd);
    emOrdem(raiz, v);
    return v;
}

void AVL::emOrdem(NoAVL *no, std::vector<int> &v) const {
    if (!no) return;
    emOrdem(no->esq, v);
    v.push_back(no->chave);
    emOrdem(no->dir, v);
}

int AVL::altura() const { return h(raiz); }

bool AVL::chaveRaiz(int &chave) const {
    if (!raiz) return false;
    chave = raiz->chave;
    return true;
}

int AVL::filhos(int chave) const {
    NoAVL *atual = raiz;
    while (atual && atual->chave != chave)
        atual = chave < atual->chave ? atual->esq : atual->dir;
    if (!atual) return -1;
    return (atual->esq != nullptr) + (atual->dir != nullptr);
}

// ---------------------------------------------------------------
// Impressao da estrutura, no formato:
//   40 (fb=0)
//   |-- E: 20 (fb=0)
//   `-- D: 60 (fb=0)
// ---------------------------------------------------------------
void AVL::imprimir(std::ostream &out, bool) const {
    if (!raiz) {
        out << "(arvore vazia)\n";
        return;
    }
    out << raiz->chave << " (fb=" << fatorBalanceamento(raiz) << ")\n";
    if (raiz->esq || raiz->dir) {
        imprimir(out, raiz->esq, "", false, "E");
        imprimir(out, raiz->dir, "", true, "D");
    }
}

void AVL::imprimir(std::ostream &out, NoAVL *no, const std::string &prefixo,
                   bool ultimo, const std::string &lado) const {
    out << prefixo << (ultimo ? "`-- " : "|-- ") << lado << ": ";
    if (!no) {
        out << "nulo\n";
        return;
    }
    out << no->chave << " (fb=" << fatorBalanceamento(no) << ")\n";
    if (no->esq || no->dir) {
        std::string novo = prefixo + (ultimo ? "    " : "|   ");
        imprimir(out, no->esq, novo, false, "E");
        imprimir(out, no->dir, novo, true, "D");
    }
}

// ---------------------------------------------------------------
// Verificacao das propriedades:
//  - ordem de busca (esq < no < dir) usando intervalo [min, max]
//  - altura guardada no no bate com a altura real
//  - fator de balanceamento entre -1 e 1
// Retorna a altura calculada ou -1 se achou erro.
// ---------------------------------------------------------------
bool AVL::verificar(std::string &erro) const {
    erro.clear();
    if (verificar(raiz, -(1LL << 40), (1LL << 40), erro) < 0) return false;
    if ((int)emOrdem().size() != qtd) {
        erro = "contador de elementos nao bate com a arvore";
        return false;
    }
    return true;
}

int AVL::verificar(NoAVL *no, long long min, long long max, std::string &erro) const {
    if (!no) return 0;
    std::ostringstream ss;
    if (no->chave <= min || no->chave >= max) {
        ss << "no " << no->chave << " quebra a ordem da arvore de busca";
        erro = ss.str();
        return -1;
    }
    int he = verificar(no->esq, min, no->chave, erro);
    if (he < 0) return -1;
    int hd = verificar(no->dir, no->chave, max, erro);
    if (hd < 0) return -1;

    int real = 1 + std::max(he, hd);
    if (real != no->altura) {
        ss << "altura guardada no no " << no->chave << " esta errada";
        erro = ss.str();
        return -1;
    }
    if (he - hd > 1 || he - hd < -1) {
        ss << "no " << no->chave << " desbalanceado (fb=" << he - hd << ")";
        erro = ss.str();
        return -1;
    }
    return real;
}
