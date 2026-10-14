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
- `tests/`: testes das estruturas, da integração e do menu.
- [EXPLICACAO_ESTRUTURAS.md](EXPLICACAO_ESTRUTURAS.md): exemplos de ponteiros,
  contratos, complexidades, regras da integração e roteiro para o vídeo.

## Compilar e executar

Na raiz do repositório, com GCC instalado (Clang aceita as mesmas opções):

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror main.c clinica.c paciente.c fila.c medico.c pilha.c -o clinica.exe
```

Executar no PowerShell ou no Linux/macOS:

```sh
./clinica.exe
```

O nome `.exe` é uma convenção; o compilador gera um binário para a plataforma
em que estiver sendo executado. O programa não precisa de bibliotecas externas.

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

No Windows, com PowerShell e GCC ou Clang:

```powershell
./tests/executar.ps1 -Compilador gcc
./tests/executar.ps1 -Compilador clang -Sanitizador
```

O script compila e executa as suítes em C, os cenários do menu e os testes de
falha de alocação. Também gera o programa em `build/normal/clinica.exe` ou
`build/asan/clinica.exe`. A versão normal pode ser aberta assim:

```powershell
./build/normal/clinica.exe
```

`-Compilador` aceita o caminho absoluto do executável. `-Sanitizador` requer
suporte a AddressSanitizer. O programa não depende do script de testes para
funcionar. Não compile os testes com `-DNDEBUG`, pois isso desativa `assert`.

Para executar os testes de integração em qualquer plataforma com GCC:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -I. clinica.c paciente.c fila.c medico.c pilha.c tests/test_integracao.c -o tests/test_integracao.exe
./tests/test_integracao.exe
```

Os testes normais e com ASan passaram no Windows com Clang 23.1.3. Foram
verificados o fluxo completo, a preservação de dados em falhas de memória,
todas as opções do menu, textos no limite e encerramento por EOF.
