#include "clinica.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* Toda entrada usa linhas completas, evitando misturar scanf com fgets.
 * Se o texto exceder o array, consumimos o restante da linha e pedimos
 * novamente; assim ele nao vira a resposta da proxima pergunta. */
static int ler_texto(const char *pergunta, char *saida, size_t capacidade)
{
    for (;;) {
        size_t tamanho;
        char *inicio;
        printf("%s", pergunta);
        fflush(stdout);
        if (fgets(saida, (int)capacidade, stdin) == NULL) {
            return 0;
        }
        tamanho = strlen(saida);
        if (tamanho > 0 && saida[tamanho - 1] == '\n') {
            saida[--tamanho] = '\0';
        } else {
            int extra = getchar();
            if (extra != '\n' && extra != EOF) {
                while (extra != '\n' && extra != EOF) {
                    extra = getchar();
                }
                puts("Texto muito longo. Tente novamente.");
                continue;
            }
        }
        /* Espacos nas extremidades nao devem formar um nome vazio ou
         * atrapalhar a conversao numerica. O meio do texto e preservado. */
        while (tamanho > 0 && isspace((unsigned char)saida[tamanho - 1])) {
            saida[--tamanho] = '\0';
        }
        inicio = saida;
        while (isspace((unsigned char)*inicio)) {
            ++inicio;
        }
        if (inicio != saida) {
            memmove(saida, inicio, strlen(inicio) + 1);
        }
        if (saida[0] != '\0') {
            return 1;
        }
        puts("O campo nao pode ficar vazio. Tente novamente.");
    }
}

static int ler_inteiro(const char *pergunta, int minimo, int maximo, int *saida)
{
    char linha[128];
    while (ler_texto(pergunta, linha, sizeof linha)) {
        char *fim;
        long valor;
        errno = 0;
        valor = strtol(linha, &fim, 10);
        if (errno != ERANGE && fim != linha && *fim == '\0' &&
            valor >= minimo && valor <= maximo) {
            *saida = (int)valor;
            return 1;
        }
        printf("Entrada invalida. Digite um inteiro entre %d e %d.\n", minimo, maximo);
    }
    return 0;
}

static const char *mensagem_paciente(ResultadoPaciente resultado)
{
    switch (resultado) {
    case PACIENTE_OK: return "Cadastro atualizado com sucesso.";
    case PACIENTE_LISTA_VAZIA: return "O cadastro esta vazio.";
    case PACIENTE_NAO_ENCONTRADO: return "Paciente nao encontrado.";
    case PACIENTE_ID_DUPLICADO: return "ID de paciente ja cadastrado.";
    case PACIENTE_SEM_MEMORIA: return "Memoria insuficiente. Cadastro preservado.";
    case PACIENTE_ARGUMENTO_INVALIDO: return "Dados de paciente invalidos.";
    }
    return "Resultado de cadastro desconhecido.";
}

static const char *mensagem_medico(ResultadoMedico resultado)
{
    switch (resultado) {
    case MEDICO_OK: return "Escala atualizada com sucesso.";
    case MEDICO_LISTA_VAZIA: return "Nenhum medico de plantao.";
    case MEDICO_ID_DUPLICADO: return "ID de medico ja cadastrado.";
    case MEDICO_SEM_MEMORIA: return "Memoria insuficiente. Escala preservada.";
    case MEDICO_ARGUMENTO_INVALIDO: return "Dados de medico invalidos.";
    }
    return "Resultado de escala desconhecido.";
}

static const char *mensagem_clinica(ResultadoClinica resultado)
{
    switch (resultado) {
    case CLINICA_OK: return "Paciente adicionado ao fim da fila.";
    case CLINICA_FILA_VAZIA: return "Nao ha pacientes na fila.";
    case CLINICA_SEM_MEDICOS: return "Cadastre um medico antes de atender.";
    case CLINICA_HISTORICO_VAZIO: return "Nao ha atendimentos no historico.";
    case CLINICA_PACIENTE_NAO_CADASTRADO: return "Paciente nao cadastrado.";
    case CLINICA_SEM_MEMORIA: return "Memoria insuficiente. Operacao cancelada sem perder dados.";
    case CLINICA_ARGUMENTO_INVALIDO: return "Dados invalidos para a operacao.";
    case CLINICA_ESTADO_INVALIDO: return "Nao foi possivel concluir a operacao.";
    }
    return "Resultado de atendimento desconhecido.";
}

static void exibir_paciente(Paciente paciente)
{
    printf("ID: %d | Nome: %s | Idade: %d | Sintoma: %s\n",
           paciente.id_paciente, paciente.nome, paciente.idade, paciente.sintoma);
}

static void exibir_atendimento(Atendimento registro)
{
    printf("Paciente: %d | Nome: %s | Medico: %d | Nome: %s\n",
           registro.paciente.id_paciente, registro.paciente.nome,
           registro.medico.id_medico, registro.medico.nome);
}

static void exibir_medico_atual(const Clinica *clinica)
{
    Medico medico;
    ResultadoMedico resultado = consultar_medico_atual(&clinica->medicos, &medico);
    if (resultado == MEDICO_OK) {
        printf("Medico atual: %d | Nome: %s\n", medico.id_medico, medico.nome);
    } else {
        puts(mensagem_medico(resultado));
    }
}

static int operacoes_paciente(Clinica *clinica, int opcao)
{
    Paciente paciente = {0};
    int id;
    ResultadoPaciente resultado;

    if (opcao == 4) {
        listar_pacientes(&clinica->pacientes, stdout);
        return 1;
    }
    if (!ler_inteiro("ID do paciente: ", 1, INT_MAX, &id)) {
        return 0;
    }
    switch (opcao) {
    case 1:
        paciente.id_paciente = id;
        if (!ler_texto("Nome: ", paciente.nome, sizeof paciente.nome) ||
            !ler_inteiro("Idade: ", 0, INT_MAX, &paciente.idade) ||
            !ler_texto("Sintoma: ", paciente.sintoma, sizeof paciente.sintoma)) {
            return 0;
        }
        puts(mensagem_paciente(inserir_paciente(&clinica->pacientes, paciente)));
        break;
    case 2:
        resultado = buscar_paciente_por_id(&clinica->pacientes, id, &paciente);
        if (resultado == PACIENTE_OK) {
            exibir_paciente(paciente);
        } else {
            puts(mensagem_paciente(resultado));
        }
        break;
    case 3:
        puts(mensagem_paciente(remover_paciente(&clinica->pacientes, id)));
        break;
    case 5:
        puts(mensagem_clinica(colocar_paciente_na_fila(clinica, id)));
        break;
    }
    return 1;
}

static int operacoes_medico(Clinica *clinica, int opcao)
{
    Medico medico = {0};
    switch (opcao) {
    case 7:
        if (!ler_inteiro("ID do medico: ", 1, INT_MAX, &medico.id_medico) ||
            !ler_texto("Nome: ", medico.nome, sizeof medico.nome)) {
            return 0;
        }
        puts(mensagem_medico(inserir_medico(&clinica->medicos, medico)));
        break;
    case 8:
        exibir_medico_atual(clinica);
        break;
    case 9:
        puts(mensagem_medico(avancar_medico(&clinica->medicos)));
        exibir_medico_atual(clinica);
        break;
    case 10:
        puts(mensagem_medico(remover_medico_atual(&clinica->medicos)));
        break;
    case 11:
        listar_medicos(&clinica->medicos, stdout);
        break;
    }
    return 1;
}

static void operacoes_atendimento(Clinica *clinica, int opcao)
{
    Atendimento registro;
    ResultadoClinica resultado;

    if (opcao == 14) {
        listar_atendimentos(&clinica->historico, stdout);
        return;
    }
    if (opcao == 15) {
        if (consultar_topo(&clinica->historico, &registro) == PILHA_OK) {
            puts("Ultimo atendimento (sem remover):");
            exibir_atendimento(registro);
        } else {
            puts(mensagem_clinica(CLINICA_HISTORICO_VAZIO));
        }
        return;
    }
    resultado = opcao == 12 ? atender_proximo_paciente(clinica, &registro)
                            : desfazer_ultimo_atendimento(clinica, &registro);
    if (resultado == CLINICA_OK) {
        puts(opcao == 12 ? "Atendimento realizado."
                        : "Atendimento desfeito. Paciente devolvido ao inicio da fila.");
        exibir_atendimento(registro);
    } else {
        puts(mensagem_clinica(resultado));
    }
}

static void exibir_menu(void)
{
    puts("\n=== CLINICA VIDASAUDAVEL ===\n"
         "1. Cadastrar paciente\n"
         "2. Buscar paciente por ID\n"
         "3. Remover paciente do cadastro\n"
         "4. Listar pacientes\n"
         "5. Adicionar paciente a fila\n"
         "6. Listar fila de espera\n"
         "7. Cadastrar medico\n"
         "8. Consultar medico atual\n"
         "9. Avancar medico\n"
         "10. Remover medico atual\n"
         "11. Listar escala de medicos\n"
         "12. Atender proximo paciente\n"
         "13. Desfazer ultimo atendimento\n"
         "14. Listar historico\n"
         "15. Consultar ultimo atendimento\n"
         "0. Sair");
}

int main(void)
{
    Clinica clinica;
    int continuar = 1;
    inicializar_clinica(&clinica);

    /* O menu coordena chamadas; cada TAD continua responsavel por seus nos.
     * EOF tambem encerra o programa, inclusive no meio de um cadastro. */
    while (continuar) {
        int opcao;
        exibir_menu();
        if (!ler_inteiro("Opcao: ", 0, 15, &opcao) || opcao == 0) {
            break;
        }
        if (opcao >= 1 && opcao <= 5) {
            continuar = operacoes_paciente(&clinica, opcao);
        } else if (opcao == 6) {
            listar_fila(&clinica.fila, stdout);
        } else if (opcao >= 7 && opcao <= 11) {
            continuar = operacoes_medico(&clinica, opcao);
        } else {
            operacoes_atendimento(&clinica, opcao);
        }
    }
    liberar_clinica(&clinica);
    puts("Programa encerrado. Memoria liberada.");
    return 0;
}
