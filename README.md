# Clínica VidaSaudável

Projeto acadêmico de Estruturas de Dados em C, da UFU.

Esta contribuição implementa a lista circular de médicos e a pilha LIFO de
atendimentos. O cadastro de pacientes, a fila FIFO e o menu serão integrados
em conjunto pela equipe. O `main.c` original foi preservado.

- `medico.h` e `medico.c`: escala circular e médico atual de plantão.
- `pilha.h` e `pilha.c`: histórico por valor e operações PUSH, POP e PEEK.
- `tests/`: testes separados do programa principal.
- [EXPLICACAO_ESTRUTURAS.md](EXPLICACAO_ESTRUTURAS.md): contratos, exemplos,
  complexidades, pendências de integração e roteiro para o vídeo.

## Compilar e testar

Na raiz do repositório, com GCC ou Clang instalado:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -I. medico.c pilha.c tests/test_estruturas.c -o tests/test_estruturas.exe
```

Executar no PowerShell:

```powershell
./tests/test_estruturas.exe
```

Executar no Linux/macOS:

```sh
./tests/test_estruturas.exe
```

O nome `.exe` é apenas uma convenção do comando; o compilador gera um binário
para a plataforma em que está sendo usado.

No Windows, o script também executa os testes de falha de alocação e contabiliza
as alocações dos dois módulos:

```powershell
./tests/executar.ps1 -Compilador gcc
./tests/executar.ps1 -Compilador clang -Sanitizador
```

`-Compilador` também aceita o caminho absoluto do executável. `-Sanitizador`
requer um compilador com suporte a AddressSanitizer. Os artefatos ficam em
`build/normal` ou `build/asan`. Não compile os testes com `-DNDEBUG`, pois isso
desativa as verificações feitas por `assert`.

O comando abaixo compila apenas o programa inicial, que ainda não usa os TADs:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic main.c -o clinica.exe
```
