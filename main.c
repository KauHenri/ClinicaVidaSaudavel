#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "paciente.h"
#include "fila.h"
#include "medico.h"
#include "pilha.h"

// funcoes auxiliares para ler dados do teclado sem travar o buffer
int ler_texto(char *buffer, int tamanho) {
    while (fgets(buffer, tamanho, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[--len] = '\0';
        } else {
            int extra = getchar();
            if (extra != '\n' && extra != EOF) {
                while ((extra = getchar()) != '\n' && extra != EOF) {}
                printf("Texto muito longo. Digite novamente: ");
                continue;
            }
        }
        while (len > 0 && isspace((unsigned char)buffer[len - 1])) {
            buffer[--len] = '\0';
        }
        size_t inicio = 0;
        while (isspace((unsigned char)buffer[inicio])) {
            inicio++;
        }
        memmove(buffer, buffer + inicio, strlen(buffer + inicio) + 1);
        if (buffer[0] != '\0') {
            return 1;
        }
        printf("O campo nao pode ficar vazio. Digite novamente: ");
    }
    buffer[0] = '\0';
    return 0;
}

int ler_inteiro(const char *msg, int minimo, int maximo) {
    char linha[64];

    printf("%s", msg);
    while (ler_texto(linha, sizeof(linha))) {
        char *fim;
        errno = 0;
        long valor = strtol(linha, &fim, 10);
        if (errno != ERANGE && fim != linha && *fim == '\0' &&
            valor >= minimo && valor <= maximo) {
            return (int)valor;
        }
        printf("Entrada invalida! Digite um inteiro entre %d e %d: ", minimo, maximo);
    }
    return -1; // fim da entrada
}

// 1. Cadastra um novo paciente na lista encadeada
void acao_cadastrar_paciente(ListaPacientes *cadastro, FilaPacientes *fila) {
    Paciente p = {0};

    printf("\n--- Novo Paciente ---\n");
    p.id_paciente = ler_inteiro("Digite o ID: ", 1, INT_MAX);
    if (p.id_paciente == -1) {
        return;
    }

    printf("Nome: ");
    if (!ler_texto(p.nome, PACIENTE_TAMANHO_NOME)) {
        return;
    }

    p.idade = ler_inteiro("Idade: ", 0, INT_MAX);
    if (p.idade == -1) {
        return;
    }

    printf("Sintoma: ");
    if (!ler_texto(p.sintoma, PACIENTE_TAMANHO_SINTOMA)) {
        return;
    }

    ResultadoPaciente res = inserir_paciente(cadastro, p);

    if (res == PACIENTE_OK) {
        printf("Paciente cadastrado com sucesso!\n");

        int enviar = ler_inteiro("Deseja colocar na fila de espera agora? (1 - Sim / 0 - Nao): ", 0, 1);
        if (enviar == 1) {
            if (enfileirar_paciente(fila, p) == FILA_OK) {
                printf("Paciente adicionado na fila de espera!\n");
            } else {
                printf("Erro ao colocar paciente na fila.\n");
            }
        }
    } else if (res == PACIENTE_ID_DUPLICADO) {
        printf("Erro: Ja existe um paciente com o ID %d!\n", p.id_paciente);
    } else if (res == PACIENTE_SEM_MEMORIA) {
        printf("Erro: Memoria cheia!\n");
    } else {
        printf("Erro ao cadastrar paciente.\n");
    }
}

// 2. Encaminha um paciente ja cadastrado para a fila
void acao_enfileirar_paciente(const ListaPacientes *cadastro, FilaPacientes *fila) {
    printf("\n--- Encaminhar para Sala de Espera ---\n");
    int id = ler_inteiro("Digite o ID do paciente: ", 1, INT_MAX);
    if (id == -1) {
        return;
    }

    Paciente p;
    if (buscar_paciente_por_id(cadastro, id, &p) != PACIENTE_OK) {
        printf("Paciente nao encontrado no cadastro!\n");
        return;
    }

    if (enfileirar_paciente(fila, p) == FILA_OK) {
        printf("Paciente %s (ID %d) entrou na fila de espera.\n", p.nome, p.id_paciente);
    } else {
        printf("Erro ao colocar na fila.\n");
    }
}

// 3. Cadastra medico na lista circular
void acao_cadastrar_medico(ListaMedicos *escala) {
    Medico m = {0};

    printf("\n--- Cadastrar Medico ---\n");
    m.id_medico = ler_inteiro("CRM / ID do medico: ", 1, INT_MAX);
    if (m.id_medico == -1) {
        return;
    }

    printf("Nome do medico: ");
    if (!ler_texto(m.nome, MEDICO_TAMANHO_NOME)) {
        return;
    }

    ResultadoMedico res = inserir_medico(escala, m);
    if (res == MEDICO_OK) {
        printf("Medico %s cadastrado no plantao!\n", m.nome);
    } else if (res == MEDICO_ID_DUPLICADO) {
        printf("Erro: Medico com esse ID ja cadastrado.\n");
    } else {
        printf("Erro ao cadastrar medico.\n");
    }
}

// 4. Realiza o atendimento do proximo da fila com o medico da vez
void acao_realizar_atendimento(ListaMedicos *escala, FilaPacientes *fila, PilhaAtendimentos *historico) {
    printf("\n--- Atendimento ---\n");

    Medico med;
    if (consultar_medico_atual(escala, &med) != MEDICO_OK) {
        printf("Nao ha medicos de plantao!\n");
        return;
    }

    Paciente pac;
    if (consultar_frente_fila(fila, &pac) != FILA_OK) {
        printf("Nenhum paciente na fila de espera!\n");
        return;
    }

    // monta o registro de atendimento
    Atendimento at;
    at.paciente = pac;
    at.medico = med;

    // salva no historico primeiro
    if (empilhar_atendimento(historico, at) != PILHA_OK) {
        printf("Erro ao registrar no historico!\n");
        return;
    }

    // retira da fila
    Paciente removido;
    desenfileirar_paciente(fila, &removido);

    // passa para o proximo medico
    avancar_medico(escala);

    printf("Atendimento realizado com sucesso!\n");
    printf("Paciente: %s | Medico: %s\n", removido.nome, med.nome);
}

// 5. Desfaz o ultimo atendimento (Ctrl+Z)
void acao_desfazer_atendimento(PilhaAtendimentos *historico, FilaPacientes *fila) {
    printf("\n--- Desfazer Ultimo Atendimento ---\n");

    Atendimento ult;
    if (consultar_topo(historico, &ult) != PILHA_OK) {
        printf("Nenhum atendimento para desfazer.\n");
        return;
    }

    // devolve paciente pro inicio da fila
    if (devolver_inicio_fila(fila, ult.paciente) != FILA_OK) {
        printf("Erro ao devolver paciente para a fila.\n");
        return;
    }

    // remove da pilha
    Atendimento tirado;
    desempilhar_atendimento(historico, &tirado);

    printf("Atendimento desfeito! Paciente %s voltou para o inicio da fila.\n", tirado.paciente.nome);
}

// 6. Passa o plantao manualmente
void acao_avancar_plantao(ListaMedicos *escala) {
    if (avancar_medico(escala) == MEDICO_OK) {
        Medico atual;
        consultar_medico_atual(escala, &atual);
        printf("Plantao avancado! Medico atual: %s\n", atual.nome);
    } else {
        printf("Nenhum medico cadastrado.\n");
    }
}

// dados iniciais para testar rapido
void carregar_dados_exemplo(ListaPacientes *cadastro, ListaMedicos *escala, FilaPacientes *fila) {
    Medico m1 = {1, "Dr. Carlos (Clinico)"};
    Medico m2 = {2, "Dra. Beatriz (Pediatra)"};
    inserir_medico(escala, m1);
    inserir_medico(escala, m2);

    Paciente p1 = {101, "Maria Silva", 28, "Dor no joelho"};
    Paciente p2 = {102, "Joao Souza", 45, "Febre"};
    inserir_paciente(cadastro, p1);
    inserir_paciente(cadastro, p2);

    enfileirar_paciente(fila, p1);
    enfileirar_paciente(fila, p2);

    printf("Dados de teste carregados com sucesso!\n");
}

int main(void) {
    ListaPacientes cadastro;
    FilaPacientes fila_espera;
    ListaMedicos escala_medicos;
    PilhaAtendimentos historico;

    // inicializa todas as estruturas
    inicializar_lista_pacientes(&cadastro);
    inicializar_fila(&fila_espera);
    inicializar_medicos(&escala_medicos);
    inicializar_pilha(&historico);

    printf("===========================================\n");
    printf("        Clinica Vida Saudavel             \n");
    printf("===========================================\n");

    int teste = ler_inteiro("Carregar dados de exemplo? (1 - Sim / 0 - Nao): ", 0, 1);
    if (teste == 1) {
        carregar_dados_exemplo(&cadastro, &escala_medicos, &fila_espera);
    }

    int opcao = -1;

    do {
        if (feof(stdin) || ferror(stdin)) {
            break;
        }
        printf("\n------------- MENU -------------\n");
        printf("1. Cadastrar paciente\n");
        printf("2. Colocar paciente na fila de espera\n");
        printf("3. Cadastrar medico no plantao\n");
        printf("4. Realizar atendimento\n");
        printf("5. Desfazer ultimo atendimento\n");
        printf("6. Avancar plantao do medico\n");
        printf("7. Mostrar fila de espera\n");
        printf("8. Mostrar escala de medicos\n");
        printf("9. Mostrar historico de atendimentos\n");
        printf("10. Mostrar todos os pacientes cadastrados\n");
        printf("0. Sair\n");
        printf("--------------------------------\n");

        opcao = ler_inteiro("Escolha uma opcao: ", 0, 10);
        if (opcao == -1) {
            break;
        }

        switch (opcao) {
            case 1:
                acao_cadastrar_paciente(&cadastro, &fila_espera);
                break;
            case 2:
                acao_enfileirar_paciente(&cadastro, &fila_espera);
                break;
            case 3:
                acao_cadastrar_medico(&escala_medicos);
                break;
            case 4:
                acao_realizar_atendimento(&escala_medicos, &fila_espera, &historico);
                break;
            case 5:
                acao_desfazer_atendimento(&historico, &fila_espera);
                break;
            case 6:
                acao_avancar_plantao(&escala_medicos);
                break;
            case 7:
                printf("\n--- Fila de Espera ---\n");
                listar_fila(&fila_espera, stdout);
                break;
            case 8:
                printf("\n--- Escala de Medicos ---\n");
                listar_medicos(&escala_medicos, stdout);
                break;
            case 9:
                printf("\n--- Historico de Atendimentos ---\n");
                listar_atendimentos(&historico, stdout);
                break;
            case 10:
                printf("\n--- Pacientes Cadastrados ---\n");
                listar_pacientes(&cadastro, stdout);
                break;
            case 0:
                printf("Encerrando...\n");
                break;
            default:
                printf("Opcao invalida!\n");
                break;
        }

    } while (opcao != 0);

    // libera a memoria de todas as estruturas
    liberar_fila(&fila_espera);
    liberar_pilha(&historico);
    liberar_medicos(&escala_medicos);
    liberar_lista_pacientes(&cadastro);

    printf("Memoria liberada. Fim do programa!\n");
    return 0;
}
