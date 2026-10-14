# Estruturas da clínica — guia de estudo e integração

## 1. O que esta contribuição entrega

O projeto agora reúne os quatro módulos: lista simples de pacientes, fila FIFO,
lista circular de médicos e pilha LIFO de atendimentos. Os arquivos de pacientes
e fila vieram da branch do colega e foram mantidos sem alterações.

`Paciente` é definido somente em `paciente.h`, com `id_paciente`, `nome[50]`,
`idade` e `sintoma[100]`. `Atendimento`, em `pilha.h`, contém um `Paciente`
e um `Medico`, ambos por valor. Assim não há ponteiros frágeis para outros nós
nem uma segunda definição de paciente.

`main.c` contém o menu e a leitura de dados. `clinica.h` e `clinica.c` agrupam
os quatro TADs e coordenam fila, escala e histórico nas operações de atender
e desfazer. Esse agrupamento não cria novos nós nem substitui qualquer TAD.

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

Esse trecho exemplifica chamadas aos TADs; o menu completo está em `main.c`.
Inicializar define o ponteiro como `NULL`. Inicialize uma vez e use a liberação
antes de reutilizar uma estrutura ocupada: reinicializar sem liberar perderia
os endereços dos nós.

Os nós são privados nos arquivos `.c`. Os cabeçalhos expõem apenas os dados,
um ponteiro para tipo incompleto e as operações. O menu deve usar as funções,
sem manipular `atual` ou `topo`. Não copie um TAD ocupado por atribuição: duas
estruturas passariam a compartilhar os mesmos nós e poderiam liberar a mesma
memória duas vezes. Essa restrição também vale para `Clinica`. Já `Paciente`,
`Medico` e `Atendimento` podem ser copiados normalmente, pois seus campos
de texto são arrays dentro da própria estrutura.

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
antigos continuam intactos. IDs são inteiros e precisam ser únicos na escala.
O TAD aceita inteiros; o menu usa IDs positivos para facilitar o cadastro.

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

O registro reutiliza os tipos públicos:

```c
typedef struct {
    Paciente paciente;
    Medico medico;
} Atendimento;
```

Por exemplo, o ID fica em `registro.paciente.id_paciente`. A atribuição de
um `Paciente` também copia nome e sintoma. Remover o cadastro ou o médico
não apaga a fotografia já guardada no histórico.

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
  rejeitadas pelos módulos de médicos e pilha antes da inserção. O menu
  também limita os textos antes de chamar o cadastro de pacientes.
- Os códigos `MEDICO_OK` e `PILHA_OK` valem zero: compare com o nome do código,
  sem tratar retorno como um booleano de sucesso.
- Os demais retornos distinguem vazio, falta de memória, argumentos inválidos
  e, na lista, ID duplicado. A função que falha preserva a estrutura.
- As consultas entregam cópias, sem expor endereços internos.
- `liberar_medicos` e `liberar_pilha` podem ser chamadas em estrutura vazia ou
  novamente após a liberação. Isso simplifica a saída do programa.
- Ponteiros para os TADs precisam apontar para estruturas inicializadas.
  Aceitar `NULL` não torna válido um ponteiro não inicializado ou já liberado.

## 6. Integração com o cadastro e a fila

`Clinica` reúne `pacientes`, `fila`, `medicos` e `historico`. Inicializar e
liberar esse agrupamento chama as quatro funções de inicialização/liberação.
As operações do menu usam as interfaces públicas, sem acessar os nós.

`colocar_paciente_na_fila` busca um ID no cadastro e passa a cópia encontrada
para `enfileirar_paciente`. Se o ID não existir ou faltar memória, a fila não muda.

### Atendimento com preservação de dados

`atender_proximo_paciente`, em `clinica.c`, segue esta ordem:

1. Consultar o paciente na frente, sem retirá-lo.
2. Consultar o médico atual. Sem médico, o paciente continua aguardando.
3. Copiar ambos para um `Atendimento` e executar PUSH.
4. Somente após o PUSH funcionar, retirar o paciente da fila.
5. Avançar o médico e devolver o registro ao chamador para exibir na tela.

O PUSH é a única etapa desse fluxo que aloca. Se faltar memória, fila, escala
e histórico continuam como estavam. A retirada e o avanço não alocam.
Existe uma proteção que desfaz o PUSH se a retirada falhar inesperadamente.
O programa é sequencial: não há outro atendimento alterando os TADs entre
essas chamadas.

### Desfazimento com preservação de dados

`desfazer_ultimo_atendimento` segue esta ordem:

1. Executar PEEK para consultar o último registro sem removê-lo.
2. Chamar `devolver_inicio_fila` com `registro.paciente`.
3. Somente após a devolução funcionar, executar POP e entregar a cópia ao menu.

Se a alocação do nó da fila falhar, o registro continua na pilha e pode ser
usado numa nova tentativa. POP não aloca memória. Se houver uma falha
inesperada no POP, a entrada recém-devolvida é retirada para restaurar a fila.

Exemplo com pacientes P1, P2, P3 e médicos Ana e Bruno:

```text
Inicial:       fila P1 -> P2 -> P3; atual Ana; historico vazio
Atender P1:    fila P2 -> P3; atual Bruno; topo P1/Ana
Atender P2:    fila P3; atual Ana; topo P2/Bruno -> P1/Ana
Desfazer P2:   fila P2 -> P3; atual Ana; topo P1/Ana
Atender P2:    fila P3; atual Bruno; topo P2/Ana -> P1/Ana
```

### Regras usadas nesta versão

- O médico avança automaticamente depois de um atendimento concluído.
- Desfazer devolve o paciente ao início da fila e mantém a posição atual da
  escala. Não tenta restaurar um médico que pode ter sido removido.
- Cada entrada da fila representa uma solicitação. A FIFO do colega permite
  várias entradas do mesmo ID; o menu mantém esse contrato. IDs no cadastro
  e na escala continuam únicos.
- Remover do cadastro não remove cópias que já estejam na fila ou no histórico.
  Desfazer restaura a entrada de espera, sem recriar automaticamente o cadastro.
- Todos os dados ficam na memória da execução e são liberados ao sair.

| Operação de integração | Custo |
|---|---|
| `inicializar_clinica` | O(1) |
| `colocar_paciente_na_fila` | O(p), devido à busca no cadastro |
| `atender_proximo_paciente` | O(1) |
| `desfazer_ultimo_atendimento` | O(1) |
| `liberar_clinica` | O(p + f + m + a) |

`p`, `f`, `m` e `a` são as quantidades de pacientes cadastrados, entradas
da fila, médicos e atendimentos. O agrupamento usa espaço auxiliar O(1).

### Leitura no menu

O menu usa `fgets` para nomes, sintomas e números. `strtol` verifica se o
número inteiro foi lido por completo e cabe no intervalo permitido.
Uma linha longa é descartada e solicitada novamente, sem contaminar a próxima
pergunta. Campos vazios são rejeitados; espaços nas extremidades são retirados.
EOF encerra o programa e libera os quatro TADs, mesmo no meio de um cadastro.

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
`malloc_teste` e `free_teste`, substituídos apenas nos objetos dos quatro TADs.
Ela força falhas de alocação em estruturas vazias e ocupadas, verifica a
recuperação e exige zero alocações pendentes ao terminar. O programa final
continua usando `malloc` e `free` da biblioteca padrão.

`tests/test_integracao.c` verifica cadastro, busca, remoções, ordem FIFO,
devolução ao início, atendimento com rotação, vários desfazimentos, cópias
após excluir cadastros e médicos e falhas de alocação sem perda de dados.

`tests/test_menu.ps1` executa o programa com entradas fornecidas por um script:
passa por todas as opções, confere as saídas em ordem, testa entradas inválidas,
overflow numérico, textos longos, tamanhos máximos e EOF com estruturas ocupadas.

O modo `-Sanitizador` executa as quatro suítes em C e os três cenários do menu
com AddressSanitizer. Todos passaram na execução normal e com ASan. A
contabilização dos TADs complementa essa verificação: não confundir ASan
com uma garantia de detecção automática de todos os vazamentos no Windows.
O alocador de teste terminou com zero alocações pendentes nos quatro TADs.

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
9. Abrir `clinica.c`: explicar PUSH antes da retirada e devolução antes do POP.
   Mostrar como a cópia permanece válida após excluir cadastro ou médico.
10. No menu, cadastrar dois pacientes e dois médicos, enfileirar, atender,
    consultar histórico e desfazer. Mostrar a prioridade da fila e a rotação.
11. Executar os testes e explicar os retornos de vazio e falha de alocação.
12. Encerrar mostrando as funções de liberação e justificando as complexidades:
    avanço O(1), inserção/remoção da lista O(n), PUSH/POP/PEEK O(1).

Para estudar, tentem desenhar os ponteiros antes e depois de cada operação
sem olhar o código. Se conseguirem explicar quais enlaces mudam e qual nó
será liberado, terão entendido os pontos principais para a apresentação.
