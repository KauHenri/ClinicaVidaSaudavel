# Clínica VidaSaudável

Projeto acadêmico de Estruturas de Dados em C, da UFU. Os quatro módulos estão
integrados em um menu de terminal: lista simples de pacientes, fila FIFO,
lista circular de médicos e pilha LIFO de atendimentos.

## Organização

- `paciente.h` e `paciente.c`: cadastro do colega, mantido sem alterações.
- `fila.h` e `fila.c`: espera FIFO do colega, mantida sem alterações.
- `medico.h` e `medico.c`: escala circular e médico atual.
- `pilha.h` e `pilha.c`: histórico com cópias de paciente e médico.
- `clinica.h` e `clinica.c`: coordenação do atendimento e desfazimento.
- `main.c`: menu e validação das entradas.
- `Makefile`: compilação, execução e testes pelo comando `make`.
- `tests/`: testes das estruturas, da integração e do menu.
- [EXPLICACAO_ESTRUTURAS.md](EXPLICACAO_ESTRUTURAS.md): exemplos de ponteiros,
  contratos, complexidades, regras da integração e roteiro para o vídeo.

## Compilar e executar

Na raiz do repositório, com GNU Make e GCC ou Clang disponíveis no `PATH`:

```sh
make
```

O **Makefile** é o arquivo de regras; **make** é a ferramenta que lê esse
arquivo. Neste projeto, o alvo padrão compila o que mudou e abre o menu.
Não é necessário chamar o executável pelo caminho da pasta de compilação.

| Comando | Ação |
|---|---|
| `make` ou `make run` | Compilar, se necessário, e abrir a clínica |
| `make build` | Somente compilar |
| `make test` | Executar os testes |
| `make test-asan` | Executar os testes com AddressSanitizer |

O Makefile prefere `gcc`, usando `clang` quando GCC não estiver disponível.
Para escolher explicitamente: `make CC=clang` ou `make CC=gcc`.
Cada `.c` gera um objeto `.o`; a ligação reúne os objetos em um programa.
As dependências dos cabeçalhos também são registradas, para recompilar quando
um `.h` mudar. Os artefatos ficam em `build/` e são ignorados pelo Git.

No Windows, algumas distribuições chamam a ferramenta de `mingw32-make`.
Nesta máquina, o comando `make` foi disponibilizado pelo GNU Make portátil.
Depois da atualização do `PATH`, abra um novo terminal; se o editor ainda
usar o ambiente antigo, reabra o editor. Em outro computador, instale GNU
Make e um compilador C. O programa continua sendo C padrão, sem bibliotecas
externas obrigatórias para sua execução.

## Usar o menu

1. Cadastrar paciente (opção 1) e médico (7).
2. Adicionar o paciente à fila pelo ID (5).
3. Atender o primeiro paciente (12); o médico avança automaticamente.
4. Consultar o último atendimento (15) ou listar o histórico (14).
5. Desfazer (13): o paciente volta ao início da fila. A escala médica mantém
   sua posição atual.
6. Sair (0): todas as estruturas são liberadas.

O menu também permite buscar, remover e listar pacientes, listar a fila,
avançar/remover o médico atual e listar a escala. IDs no menu são positivos;
idade aceita inteiros não negativos. Nomes aceitam até 49 bytes e sintomas
até 99 bytes, reservando espaço para o terminador. Campos vazios e entradas
fora dos limites são solicitados novamente.

Cada entrada na fila é uma solicitação; um paciente pode ter várias entradas,
conforme o contrato da FIFO. Remover o cadastro não apaga cópias da fila ou do
histórico. Os dados ficam na memória e não são persistidos ao sair.

## Testes

Na raiz do repositório:

```sh
make test
make test-asan CC=clang
```

`make test` executa as quatro suítes em C, incluindo falhas simuladas de
alocação. No Windows, também executa os três cenários do menu pelo PowerShell.
Em Linux/macOS, esse alvo executa as suítes em C; os cenários do menu usam
o script PowerShell fornecido para Windows.

`make test-asan` usa uma pasta separada e requer um compilador com suporte
a AddressSanitizer. No Windows, as DLLs do ASan precisam estar disponíveis no
`PATH` do compilador. Não use `-DNDEBUG` nos testes, pois isso desativa `assert`.
O script anterior `tests/executar.ps1` foi preservado como alternativa técnica;
os comandos recomendados do projeto agora são os alvos do Makefile.

Os alvos de execução, compilação e testes foram verificados com GNU Make 4.4.1.
Os testes normais e com ASan passaram no Windows com Clang 23.1.3. Foram
verificados o fluxo completo, a preservação de dados em falhas de memória,
todas as opções do menu, textos no limite e encerramento por EOF.
