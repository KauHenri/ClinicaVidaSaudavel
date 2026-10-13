# Lista circular e pilha LIFO — guia de estudo

## 1. O que esta contribuição entrega

Foram implementados somente os módulos de médicos e de atendimentos.
O repositório de origem tinha `main.c`, `.gitignore` e configurações do VS Code;
não tinha `tipos.h`, `paciente.h`, `paciente.c`, `fila.h` ou `fila.c`.
O `main.c` já contém um `Paciente` com `id_paciente`, `nome[50]`, `idade` e
`sintoma[100]`, além de um nó inicial do cadastro. Esse arquivo foi preservado.

Não criamos outro tipo chamado `Paciente` nem um contrato definitivo para a
fila. `Atendimento`, em `pilha.h`, guarda uma fotografia dos dados existentes
do paciente e uma cópia de `Medico`. Isso basta para consultar o histórico e
recuperar os dados no desfazimento sem depender de endereços de outras listas.

## 2. Como usar os TADs

```c
#include "medico.h"
#include "pilha.h"

ListaMedicos escala;
PilhaAtendimentos historico;
inicializar_medicos(&escala);
inicializar_pilha(&historico);

/* Conferir cada retorno antes de continuar o fluxo de atendimento. */
ResultadoMedico resultado = inserir_medico(&escala, (Medico){1, "Ana"});
if (resultado == MEDICO_OK) {
    listar_medicos(&escala, stdout);
}

/* Na saida do programa, inclusive em caminhos de erro: */
liberar_pilha(&historico);
liberar_medicos(&escala);
```

Esse trecho exemplifica chamadas para o futuro menu; não é um novo `main.c`.
Inicializar define o ponteiro como `NULL`. Inicialize uma vez e use a liberação
antes de reutilizar uma estrutura ocupada: reinicializar sem liberar perderia
os endereços dos nós.

Os nós são privados nos arquivos `.c`. Os cabeçalhos expõem apenas os dados,
um ponteiro para tipo incompleto e as operações. O menu deve usar as funções,
sem manipular `atual` ou `topo`. Não copie um TAD ocupado por atribuição: duas
estruturas passariam a compartilhar os mesmos nós e poderiam liberar a mesma
memória duas vezes. Já `Medico` e `Atendimento` podem ser copiados normalmente.

## 3. Lista simplesmente encadeada circular

Cada `NoMedico` contém `Medico dados` e `NoMedico *prox`.
`dados` guarda ID e nome; `prox` guarda o endereço do próximo nó.
O último nó aponta para o primeiro, formando um círculo. Não há um primeiro
fixo: `ListaMedicos.atual` indica de onde começa a escala neste momento.

```text
atual
  |
  v
 Ana -> Bruno -> Clara
  ^               |
  +---------------+
```

### Inserir e avançar, passo a passo

1. A lista vazia tem `atual == NULL`.
2. Ao inserir Ana, alocamos um nó; `Ana.prox` aponta para Ana e `atual` aponta
   para Ana. Um único médico também forma uma lista circular.
3. Ao inserir Bruno, procuramos o último nó. Ligamos `Bruno.prox` a Ana e
   `Ana.prox` a Bruno. O atual continua sendo Ana.
4. Ao inserir Clara, ligamos `Clara.prox` a Ana e `Bruno.prox` a Clara.
5. Cada avanço executa `atual = atual->prox`: Ana, Bruno, Clara, Ana...

Antes da alocação, uma volta completa procura IDs duplicados. Depois do
`malloc`, verificamos se o endereço é `NULL`. Se faltar memória, os enlaces
antigos continuam intactos. IDs são inteiros e precisam ser únicos na escala;
não impomos uma faixa positiva que ainda não foi combinada pela equipe.

`malloc(sizeof *novo)` reserva espaço para o nó inteiro. `sizeof *novo` mede
o tipo do objeto apontado, sem precisar acessar um nó já existente; `novo`
guarda o endereço do bloco retornado. Usar apenas o tamanho do ponteiro não
reservaria espaço suficiente para os dados e a ligação do nó.

Inserir significa acrescentar ao fim da escala **a partir do atual**. Se o
atual for Bruno e a escala for Bruno → Clara → Ana → Bruno, inserir Daniel
produz Bruno → Clara → Ana → Daniel → Bruno. Bruno continua de plantão.
Essa política está explícita para evitar expectativas diferentes na integração.

### Remover o atual, passo a passo

Considere Ana → Bruno → Clara → Ana, com Bruno atual:

1. Guardamos o endereço de Bruno em `removido`.
2. Procuramos o antecessor: Ana, pois `Ana.prox` aponta para Bruno.
3. Fazemos `Ana.prox = Bruno.prox`, conectando Ana diretamente a Clara.
4. Fazemos `atual = Bruno.prox`; Clara assume o plantão.
5. Liberamos Bruno com `free(removido)`.

Com um só nó, `atual->prox == atual`: basta deixar `atual = NULL` e liberar
o nó. Em lista vazia, a operação retorna `MEDICO_LISTA_VAZIA`.

### Listagem e liberação

Uma lista circular não chega a `NULL` no percurso normal. Por isso usamos
`do ... while (cursor != lista->atual)`: o atual é visitado uma vez e a
listagem termina quando o cursor retorna a ele. `[ATUAL]` marca o plantonista.
O cursor local não muda a escala.

Para destruir a estrutura, encontramos o último nó e colocamos seu `prox`
em `NULL`. Essa abertura acontece apenas durante a liberação; o TAD em uso
continua circular. Em seguida percorremos os nós, sempre guardando `seguinte`
antes de chamar `free`. Ao terminar, `atual == NULL`. Essa estratégia evita
comparar um ponteiro com um nó que já foi liberado.

### Operações da lista

| Função | Responsabilidade | Custo |
|---|---|---|
| `inicializar_medicos` | Definir estado vazio | O(1) |
| `inserir_medico` | Verificar ID e inserir ao fim da escala | O(n) |
| `avancar_medico` | Trocar atual por seu próximo | O(1) |
| `consultar_medico_atual` | Copiar dados do atual | O(1) |
| `remover_medico_atual` | Encontrar antecessor e remover | O(n), O(1) com um nó |
| `listar_medicos` | Percorrer uma volta sem alterar o atual | O(n) |
| `liberar_medicos` | Abrir o círculo e liberar os nós | O(n) |

`n` é o número de médicos. Inserção e remoção percorrem a lista porque guardamos
apenas o atual, sem um ponteiro adicional para o último ou para o antecessor.
O espaço total é O(n); cada operação usa O(1) de espaço auxiliar.

## 4. Pilha LIFO de atendimentos

LIFO significa *Last In, First Out*: o último a entrar é o primeiro a sair.
Cada `NoAtendimento` guarda `Atendimento dados` e um ponteiro `prox` para o
atendimento anterior. `PilhaAtendimentos.topo` aponta para o mais recente.
O último nó no percurso, que é o registro mais antigo, aponta para `NULL`.

### PUSH e POP, passo a passo

```text
Inicial:       topo -> NULL
PUSH A:        topo -> A -> NULL
PUSH B:        topo -> B -> A -> NULL
PUSH C:        topo -> C -> B -> A -> NULL
POP devolve C: topo -> B -> A -> NULL
POP devolve B: topo -> A -> NULL
POP devolve A: topo -> NULL
```

No PUSH, alocamos `novo`, copiamos os dados, fazemos `novo->prox = topo`
e só então `topo = novo`. A mesma sequência funciona em pilha vazia.
Falha de alocação retorna `PILHA_SEM_MEMORIA` sem modificar o topo.

No POP, verificamos se há topo, guardamos o nó, copiamos seus dados para
`*saida`, atualizamos o topo para `removido->prox` e liberamos o nó.
Os dados retornados continuam válidos porque pertencem à variável do chamador.
O nó precisa ser liberado porque deixou de fazer parte da pilha.

PEEK, implementado em `consultar_topo`, também copia os dados, mas não muda
enlaces nem libera memória. Alterar a cópia recebida não altera o histórico.
POP e PEEK exigem uma saída não nula. Em pilha vazia retornam `PILHA_VAZIA`
e deixam o conteúdo da saída intacto.

A listagem percorre do topo até `NULL`, do mais recente ao mais antigo,
marcando o primeiro com `[TOPO]`. A liberação avança o topo antes de liberar
cada nó; ao terminar, `topo == NULL`.

### Operações da pilha

| Função | Responsabilidade | Custo |
|---|---|---|
| `inicializar_pilha` | Definir estado vazio | O(1) |
| `empilhar_atendimento` | PUSH de uma cópia dos dados | O(1) |
| `desempilhar_atendimento` | POP e entrega dos dados removidos | O(1) |
| `consultar_topo` | PEEK sem remover | O(1) |
| `listar_atendimentos` | Consultar todo o histórico | O(n) |
| `liberar_pilha` | Liberar todos os nós | O(n) |

Aqui `n` é o número de atendimentos. PUSH, POP e PEEK acessam somente o topo;
as cópias têm tamanho fixo. A pilha ocupa O(n) e usa O(1) de espaço auxiliar.

## 5. Contratos e cuidados com memória

- Todo `malloc` tem retorno conferido e todo nó removido é liberado.
- As strings devem conter `\0` dentro dos arrays: 50 bytes para nomes e
  100 para sintoma, incluindo o terminador. Entradas sem terminador são
  rejeitadas antes da inserção. Isso evita leituras além do array ao listar.
- Os códigos `MEDICO_OK` e `PILHA_OK` valem zero: compare com o nome do código,
  sem tratar retorno como um booleano de sucesso.
- Os demais retornos distinguem vazio, falta de memória, argumentos inválidos
  e, na lista, ID duplicado. A função que falha preserva a estrutura.
- As consultas entregam cópias, sem expor endereços internos.
- `liberar_medicos` e `liberar_pilha` podem ser chamadas em estrutura vazia ou
  novamente após a liberação. Isso simplifica a saída do programa.
- Ponteiros para os TADs precisam apontar para estruturas inicializadas.
  Aceitar `NULL` não torna válido um ponteiro não inicializado ou já liberado.

## 6. Como integrar com o cadastro e a fila

Ainda é necessário combinar as interfaces dos módulos do colega e fazer o
menu em conjunto. Nenhuma fila foi implementada nesta contribuição.

Com o `Paciente` atual do `main.c`, a fotografia pode ser montada assim:

```c
/* p e uma copia valida do paciente e medico foi obtido pela consulta. */
Atendimento registro;
registro.id_paciente = p.id_paciente;
memcpy(registro.nome_paciente, p.nome, sizeof registro.nome_paciente);
registro.idade_paciente = p.idade;
memcpy(registro.sintoma, p.sintoma, sizeof registro.sintoma);
registro.medico = medico;
```

Esse exemplo requer `<string.h>` e os mesmos tamanhos de campos do código
atual. Não é uma função pronta do cadastro nem deve ser incluído fora de um
contexto em que `p` e `medico` estejam definidos. Para reconstruir o paciente,
copie os quatro campos de volta ao tipo usado pelo módulo de fila.

Para um atendimento normal, o fluxo sugerido é:

1. Verificar médico atual e paciente na frente da fila.
2. Montar o registro e tentar empilhá-lo.
3. Somente após o PUSH bem-sucedido, retirar o paciente da fila.
4. Se a retirada falhar, remover o registro recém-empilhado para restaurar
   a consistência. Avançar o médico somente após concluir o atendimento.

Isso depende de a fila permitir consultar a frente sem remover. Se o módulo
do colega só permitir retirar, será necessário combinar uma restauração
segura em caso de falha do PUSH. Não se deve perder o paciente nem criar
histórico de um atendimento que não foi concluído.

Para desfazer, a sequência recomendada é:

1. Usar PEEK para copiar o último atendimento sem removê-lo.
2. Reconstruir o paciente e tentar devolvê-lo ao início da fila.
3. Só depois de a reinserção funcionar, executar POP.

Assim uma falha de alocação na fila não elimina o registro necessário para
repetir o desfazimento. Com uma pilha inicializada, saída válida e sem mudanças
entre essas chamadas, POP não aloca memória e remove o mesmo registro consultado.

Decisões pendentes para a equipe:

- A fila terá uma operação para devolver ao início? Essa é a política sugerida
  para preservar a prioridade anterior. Não simulem uma segunda fila.
- O desfazimento também deve restaurar o médico anterior da rotação? Os módulos
  atuais não desfazem o avanço; decidam isso em conjunto no menu.
- O que fazer se o cadastro permanente do paciente foi removido após atender?
  A fotografia recupera os dados, mas a regra do cadastro precisa ser combinada.
- Como evitar duplicar na fila um paciente cujo atendimento está sendo desfeito?
- Quando modularizarem `Paciente`, podem movê-lo para um cabeçalho compartilhado
  e adaptar o histórico conjuntamente. Nenhum `tipos.h` provisório foi imposto.

## 7. Testes reproduzíveis

Veja os comandos no `README.md`. `tests/test_estruturas.c` verifica:

- Lista vazia, primeira inserção, vários médicos e ID duplicado.
- Oito voltas completas, inserção após rotação e circularidade após remoções.
- Listagem com conteúdo exato, ordem correta e manutenção do atual.
- Remoção até ficar vazia e liberação com zero, um e vários nós.
- PUSH de um e vários registros, PEEK sem remover e POP em ordem inversa.
- Cópias independentes, dados completos após POP e manutenção da saída em falhas.
- Histórico preservado após remover o médico da escala e liberar os TADs.
- Histórico com conteúdo exato e manutenção do topo após listar.
- Argumentos nulos e strings sem terminador.

O script PowerShell compila também uma versão instrumentada com
`malloc_teste` e `free_teste`, substituídos apenas nos objetos dos dois TADs.
Ela força falhas de alocação em estruturas vazias e ocupadas, verifica a
recuperação e exige zero alocações pendentes ao terminar. O programa final
continua usando `malloc` e `free` da biblioteca padrão.

O modo `-Sanitizador` executa ambas as versões com AddressSanitizer. A
contabilização dos TADs complementa essa verificação: não confundir ASan
com uma garantia de detecção automática de todos os vazamentos no Windows.
Os testes não executam o `main.c` original nem validam os futuros módulos de fila.

## 8. Roteiro técnico para o vídeo

1. Apresentar o fluxo da clínica e a responsabilidade de cada estrutura.
2. Abrir `medico.h`: mostrar os dados e explicar o significado de `atual`.
3. Abrir `medico.c`: desenhar três nós e destacar a ligação do último ao primeiro.
4. Explicar a primeira inserção, a busca do último e a atualização dos enlaces.
5. Demonstrar uma volta completa da escala e explicar a parada da listagem.
6. Remover o atual: mostrar o antecessor e o próximo assumindo o plantão;
   depois mostrar a remoção do único nó.
7. Abrir `pilha.h` e `pilha.c`: explicar a fotografia do atendimento e o topo.
8. Fazer PUSH de A, B e C; demonstrar PEEK e POP devolvendo C, B e A.
9. Mostrar por que a cópia retornada continua válida após `free` e como isso
   prepara o desfazimento, respeitando uma eventual falha de reinserção na fila.
10. Executar os testes e explicar os retornos de vazio e falha de alocação.
11. Encerrar mostrando as funções de liberação e justificando as complexidades:
    avanço O(1), inserção/remoção da lista O(n), PUSH/POP/PEEK O(1).

Para estudar, tentem desenhar os ponteiros antes e depois de cada operação
sem olhar o código. Se conseguirem explicar quais enlaces mudam e qual nó
será liberado, terão entendido os pontos principais para a apresentação.
