# Árvores Balanceadas: AVL e Rubro-Negra

Trabalho da Unidade II de Estruturas de Dados Básicas II (IMD0039), "Amazon Engineering Challenge".

**Autor:** Leonardo Souza  
**Repositório:** https://github.com/leosouza7oliveira/arvores-balanceadas

A ideia do trabalho é simular o sistema de logística da Amazon na Black Friday, que precisa registrar, localizar e remover IDs de encomendas rapidamente. Como os IDs de pedidos costumam ser gerados em sequência, uma árvore de busca binária comum degeneraria numa lista. Por isso implementei as duas árvores balanceadas vistas na disciplina e comparei as duas com os mesmos dados.

## Estrutura do repositório

```
arvores-balanceadas/
├── include/
│   ├── arvore.h          classe base com as operações comuns
│   ├── avl.h
│   └── rubronegra.h
├── src/
│   ├── avl.cpp           implementação da AVL
│   ├── rubronegra.cpp    implementação da Rubro-Negra
│   ├── main.cpp          programa principal (menu)
│   ├── testes.cpp        testes automáticos
│   └── comparacao.cpp    experimentos da análise comparativa
├── bin/                  programas já compilados (macOS, Apple Silicon)
├── exemplos/             roteiros de entrada para demonstração
├── resultados/           saídas dos testes, da comparação e CSVs
└── Makefile
```

## Dependências

Só precisa de um compilador C++ com suporte a C++17 (g++ 7+ ou clang 5+). Não usei nenhuma biblioteca externa, e as árvores não usam `std::map` nem `std::set` (o `std::set` aparece apenas nos testes, como gabarito para conferir as respostas).

## Como compilar

Com `make` instalado:

```bash
make
```

Isso gera três executáveis em `bin/`: `arvores`, `testes` e `comparacao`.

Sem `make` (por exemplo no Windows com MinGW), dá para compilar direto:

```bash
g++ -std=c++17 -O2 -Iinclude src/main.cpp src/avl.cpp src/rubronegra.cpp -o bin/arvores
g++ -std=c++17 -O2 -Iinclude src/testes.cpp src/avl.cpp src/rubronegra.cpp -o bin/testes
g++ -std=c++17 -O2 -Iinclude src/comparacao.cpp src/avl.cpp src/rubronegra.cpp -o bin/comparacao
```

Os arquivos que estão em `bin/` foram compilados no macOS (ARM). Em Linux ou Windows é preciso recompilar com um dos comandos acima.

## Como executar

### Programa principal

```bash
./bin/arvores
```

Ao abrir, o programa pergunta qual árvore usar (1 = AVL, 2 = Rubro-Negra). Depois aparece o menu:

```
------ Arvore AVL (0 elementos, altura 0) ------
  1 - Inserir encomenda (ID)
  2 - Remover encomenda
  3 - Buscar encomenda
  4 - Listar IDs em ordem crescente
  5 - Mostrar estrutura da arvore
  6 - Inserir varios IDs de uma vez
  7 - Estatisticas (altura, rotacoes...)
  8 - Verificar propriedades da arvore
  9 - Trocar tipo de arvore
  0 - Sair
```

Opções de linha de comando:

- `--sem-cor`: desliga as cores ANSI (útil no prompt antigo do Windows). Mesmo sem cor, os nós da Rubro-Negra aparecem marcados com `[V]` (vermelho) ou `[P]` (preto).
- `--eco`: repete na tela o que foi lido. Serve para rodar os roteiros da pasta `exemplos/` e ver a sessão completa.

```bash
./bin/arvores --eco < exemplos/demo_avl.txt
./bin/arvores --eco < exemplos/demo_rubronegra.txt
```

### Testes automáticos

```bash
./bin/testes
```

### Comparação AVL x Rubro-Negra

```bash
./bin/comparacao
```

Demora uns 10 segundos (vai até 1 milhão de elementos). Imprime as tabelas e salva os CSVs em `resultados/`.

## Funcionalidades

As duas árvores implementam a mesma interface (`ArvoreBusca`, em `include/arvore.h`):

| Operação | Descrição |
|---|---|
| `inserir(id)` | insere e rebalanceia; retorna `false` se o ID já existe |
| `remover(id)` | remove e rebalanceia; retorna `false` se o ID não existe |
| `buscar(id)` | retorna se o ID está na árvore e conta as comparações feitas |
| `emOrdem()` | percurso em ordem (IDs em ordem crescente) |
| `imprimir()` | desenha a árvore no terminal |
| `verificar()` | confere todas as propriedades da estrutura |
| `altura()`, `tamanho()` | usados nas estatísticas |

Exemplo da visualização na AVL (mostra o fator de balanceamento de cada nó):

```
30 (fb=0)
|-- E: 20 (fb=0)
|   |-- E: 10 (fb=0)
|   `-- D: 25 (fb=0)
`-- D: 40 (fb=-1)
    |-- E: nulo
    `-- D: 50 (fb=0)
```

E na Rubro-Negra (`[V]` vermelho, `[P]` preto; no terminal os vermelhos também aparecem em vermelho):

```
20 [P]
|-- E: 10 [P]
`-- D: 40 [V]
    |-- E: 30 [P]
    |   |-- E: 25 [V]
    |   `-- D: nulo
    `-- D: 50 [P]
```

`E` é o filho esquerdo e `D` o direito. Quando um nó só tem um filho, o outro aparece como `nulo` para não ficar ambíguo de que lado ele está.

## Decisões de implementação

- **Chaves:** os IDs são `int`. A entrada é validada, então texto, número com letra no meio ou valor fora do intervalo do `int` são recusados com uma mensagem.
- **IDs repetidos:** não são inseridos. A função retorna `false`, a árvore não muda e o programa avisa o usuário. Achei que fazia sentido porque um ID de encomenda é único.
- **Busca e remoção de ID inexistente:** retornam `false` sem alterar nada, e o programa mostra "não encontrada".
- **Remoção de nó com dois filhos:** nas duas árvores o nó é substituído pelo **sucessor** (menor elemento da subárvore direita). Usei a mesma regra nas duas para a comparação ser justa.
- **AVL:** recursiva. Cada nó guarda a própria altura, e o fator de balanceamento é `altura(esq) - altura(dir)`. Na volta da recursão, a função `balancear` trata os casos LL, RR (rotação simples) e LR, RL (rotação dupla).
- **Rubro-Negra:** iterativa, seguindo o Cormen (cap. 13), com nó sentinela `NIL` preto no lugar de `nullptr` e ponteiro para o pai. A correção depois da inserção trata os 3 casos (tio vermelho, tio preto em "zigue-zague" e tio preto em linha) e a remoção trata os 4 casos do irmão.
- **Altura:** medida em níveis. Árvore vazia tem altura 0 e uma árvore só com a raiz tem altura 1.
- **Contagem de rotações:** cada rotação simples conta 1. Uma rotação dupla da AVL conta 2 (são duas rotações) e também é contada à parte em `rotacoesDuplas`.
- **Recolorações (Rubro-Negra):** conta cada vez que um nó realmente muda de cor. Pintar de preto um nó que já era preto não conta.
- **Interface comum:** o `main` trabalha com um ponteiro para a classe base `ArvoreBusca`, por isso o usuário pode escolher a árvore na hora da execução e até trocar no meio (opção 9, que pode copiar os IDs para a nova árvore).
- **Verificação:** cada árvore tem um `verificar()` que percorre tudo e confere a ordem da árvore de busca e as regras de balanceamento (alturas e `|fb| <= 1` na AVL; raiz preta, sem vermelho com filho vermelho e mesma altura negra em todos os caminhos na Rubro-Negra). Os testes chamam essa função depois de cada operação.

## Testes

`src/testes.cpp` roda exatamente os mesmos cenários nas duas árvores:

1. árvore vazia (buscar, remover, percorrer);
2. árvore com um elemento (inserir, buscar, remover até esvaziar);
3. tentativa de inserir valores repetidos;
4. os quatro casos básicos de rotação com 3 elementos;
5. inserção em ordem crescente, decrescente e aleatória (1000 elementos), verificando as propriedades a cada inserção;
6. busca de IDs existentes e inexistentes;
7. remoção de folha, de nó com 1 filho, de nó com 2 filhos e da raiz;
8. remoção de IDs inexistentes;
9. remoção de todos os elementos em ordem aleatória;
10. estresse com 20.000 operações aleatórias, comparando cada resposta com o `std::set`.

Resultado atual: **78 de 78 testes passaram** (saída completa em `resultados/saida_testes.txt`).

## Resultados da comparação (resumo)

Saída completa em `resultados/saida_comparacao.txt` e nos CSVs.

Inserindo 1.000.000 de IDs:

| Ordem | Altura AVL | Altura RN | Altura mínima | Rotações AVL | Rotações RN |
|---|---|---|---|---|---|
| crescente | 20 | 37 | 20 | 999.980 | 999.963 |
| aleatória | 24 | 24 | 20 | 698.951 | 583.488 |

Para comparação, a ABB comum com 10.000 IDs crescentes ficou com altura 10.000.

Simulação da Black Friday (1 milhão de operações: 50% pedidos novos com IDs crescentes, 35% rastreios, 15% entregas):

| | AVL | Rubro-Negra |
|---|---|---|
| Altura final (351.055 IDs) | 19 | 30 |
| Rotações | 521.199 | 526.407 |
| Comparações por busca | 16,21 | 16,42 |
| Tempo total | ~104 ms | ~82 ms |

Os tempos dependem da máquina (os meus foram num Mac com processador ARM) e variam um pouco a cada execução, mas a diferença entre as duas se manteve parecida nas vezes que rodei.

## Referências

- CORMEN, T. H. et al. *Algoritmos: Teoria e Prática*. 3. ed. Rio de Janeiro: Elsevier, 2012. Cap. 12 e 13.
- SZWARCFITER, J. L.; MARKENZON, L. *Estruturas de Dados e Seus Algoritmos*. 3. ed. Rio de Janeiro: LTC, 2010.
- ADELSON-VELSKY, G. M.; LANDIS, E. M. An algorithm for the organization of information. *Soviet Mathematics Doklady*, v. 3, 1962.
- GUIBAS, L. J.; SEDGEWICK, R. A dichromatic framework for balanced trees. *19th Annual Symposium on Foundations of Computer Science*, 1978.
- Material de aula da disciplina IMD0039.
