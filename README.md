# Clínica VidaSaudável

Sistema de atendimento de uma clínica desenvolvido em **C**, como atividade da
disciplina de **Estruturas de Dados** do curso de Engenharia de Computação da
**Universidade Federal de Uberlândia (UFU)**.

O projeto integra quatro tipos abstratos de dados em um menu de terminal:
lista encadeada simples de pacientes, fila de espera FIFO, lista circular de
médicos e pilha LIFO de atendimentos. A implementação utiliza ponteiros,
alocação dinâmica e módulos separados em arquivos `.h` e `.c`.

[Versão final v1.0.0](https://github.com/KauHenri/ClinicaVidaSaudavel/releases/tag/v1.0.0)

## Estruturas de dados

| Estrutura | Responsabilidade | Operações principais |
|---|---|---|
| Lista encadeada simples | Cadastro de pacientes | Inserção, busca por ID, remoção e listagem |
| Fila FIFO | Sala de espera por ordem de chegada | Enfileirar, consultar a frente, desenfileirar e devolver ao início |
| Lista circular | Escala rotativa de médicos | Inserção, consulta do atual, avanço, remoção do atual e listagem |
| Pilha LIFO | Histórico de atendimentos | Empilhar, consultar o topo, desempilhar e listar |

Na fila, o primeiro paciente a entrar é o primeiro a ser atendido. Na escala,
o último médico aponta novamente para o primeiro, permitindo o rodízio.
No histórico, o atendimento mais recente fica no topo e é o primeiro a ser
desfeito.

## Compilação e execução

É necessário ter **GNU Make** e um compilador **GCC ou Clang** disponíveis no
`PATH`. O código utiliza o padrão C11 e a biblioteca padrão de C.

Na pasta do projeto, execute:

```sh
make
```

O alvo padrão compila os arquivos necessários e abre o menu da clínica.

| Comando | Ação |
|---|---|
| `make` ou `make run` | Compilar e executar |
| `make build` | Somente compilar |
| `make test` | Executar os testes |
| `make test-asan CC=clang` | Executar os testes com AddressSanitizer |

O compilador pode ser escolhido com `make CC=gcc` ou `make CC=clang`.
No Windows, a ferramenta também pode estar disponível como `mingw32-make`.
Os arquivos gerados ficam em `build/`, diretório ignorado pelo Git.

## Utilização

Ao iniciar, o programa apresenta a pergunta:

```text
Carregar dados de exemplo? (1 - Sim / 0 - Nao):
```

- **Opção 0:** inicia com as estruturas vazias, permitindo cadastrar pacientes
  e médicos manualmente.
- **Opção 1:** carrega os pacientes Maria Silva e João Souza, já enfileirados,
  e os médicos Dr. Carlos e Dra. Beatriz para demonstração do atendimento.

### Opções do menu

| Opção | Operação |
|---|---|
| 1 | Cadastrar paciente e escolher se ele deve entrar na fila |
| 2 | Colocar um paciente cadastrado na fila pelo ID |
| 3 | Cadastrar médico no plantão |
| 4 | Realizar o próximo atendimento |
| 5 | Desfazer o último atendimento |
| 6 | Avançar o plantão manualmente |
| 7 | Mostrar a fila de espera |
| 8 | Mostrar a escala de médicos |
| 9 | Mostrar o histórico de atendimentos |
| 10 | Mostrar todos os pacientes cadastrados |
| 0 | Encerrar e liberar a memória |

As operações de busca e remoção do cadastro, remoção do médico atual e
consulta ao topo também estão disponíveis nas interfaces dos TADs e são
verificadas pelos testes.

### Fluxo de atendimento

1. Cadastrar os pacientes e os médicos.
2. Encaminhar os pacientes à fila de espera.
3. Consultar o primeiro paciente e o médico atual.
4. Registrar o atendimento na pilha antes de retirar o paciente da fila.
5. Retirar o paciente atendido e avançar para o próximo médico.
6. Ao desfazer, devolver o paciente ao início da fila e remover o registro
   correspondente do topo da pilha.

A ordem das operações preserva os dados caso falte memória para registrar
um atendimento ou devolver um paciente à fila. No desfazimento, a escala
médica mantém sua posição atual.

### Regras e validações

- IDs de pacientes e médicos são únicos em seus respectivos cadastros.
- O menu aceita IDs positivos e idades inteiras não negativas.
- Nomes aceitam até 49 bytes e sintomas até 99 bytes, reservando espaço para
  o terminador da string. Campos vazios, textos longos e números inválidos
  são solicitados novamente.
- Cada entrada na fila representa uma solicitação de atendimento; um mesmo
  paciente pode ter mais de uma solicitação.
- O atendimento mantém o paciente no cadastro. A fila e o histórico guardam
  cópias dos dados, independentes dos nós do cadastro e da escala.
- Os dados permanecem em memória durante a execução. Ao sair, todos os nós
  são liberados.

## Demonstrações em vídeo

As gravações em MP4 estão disponíveis nos anexos da
[versão final](https://github.com/KauHenri/ClinicaVidaSaudavel/releases/tag/v1.0.0).

### Caso 01: início com estruturas vazias

Demonstração iniciada com a **opção 0** na pergunta sobre os dados de exemplo.
Apresenta a utilização do sistema a partir do cadastro manual dos dados.

[Abrir CASO01.mp4](https://github.com/KauHenri/ClinicaVidaSaudavel/releases/download/v1.0.0/CASO01.mp4)

### Caso 02: início com dados de exemplo

Demonstração iniciada com a **opção 1**, utilizando os pacientes e médicos
carregados pelo programa para apresentar o fluxo de atendimento, a rotação
da escala e o desfazimento pelo histórico.

[Abrir CASO02.mp4](https://github.com/KauHenri/ClinicaVidaSaudavel/releases/download/v1.0.0/CASO02.mp4)

## Organização dos arquivos

| Arquivo ou diretório | Conteúdo |
|---|---|
| `main.c` | Menu, leitura das entradas e chamadas aos TADs |
| `paciente.h` e `paciente.c` | Lista encadeada de pacientes |
| `fila.h` e `fila.c` | Fila de espera FIFO |
| `medico.h` e `medico.c` | Lista circular de médicos |
| `pilha.h` e `pilha.c` | Pilha de atendimentos |
| `clinica.h` e `clinica.c` | Funções de integração utilizadas nos testes |
| `Makefile` | Regras de compilação, execução e testes |
| `tests/` | Testes das estruturas, integração, menu e falhas de alocação |

## Testes

```sh
make test
make test-asan CC=clang
```

Os testes verificam inserções, buscas, remoções, circularidade, ordem FIFO,
ordem LIFO, cópias dos dados e integração dos atendimentos. Também cobrem
estruturas vazias e falhas simuladas de alocação, verificando a preservação
do estado e a liberação dos nós.

No Windows, `make test` executa ainda os cenários do menu por PowerShell,
incluindo entradas inválidas, limites de texto e encerramento por fim da
entrada (EOF). Em Linux e macOS, o alvo executa as suítes em C.

A execução com AddressSanitizer requer um compilador compatível e suas
bibliotecas de execução disponíveis no ambiente. Os testes utilizam `assert`
e devem ser compilados com as asserções habilitadas.

## Complexidade das operações

| Estrutura | Operação | Custo |
|---|---|---|
| Lista de pacientes | Inserir, buscar ou remover por ID | O(n) |
| Fila | Enfileirar, desenfileirar, consultar a frente ou devolver ao início | O(1) |
| Lista circular | Avançar ou consultar o médico atual | O(1) |
| Lista circular | Inserir ou remover o médico atual | O(n) |
| Pilha | Empilhar, desempilhar ou consultar o topo | O(1) |
| Todas | Listar ou liberar todos os nós | O(n) |

Em cada linha, `n` representa a quantidade de nós da estrutura correspondente.
