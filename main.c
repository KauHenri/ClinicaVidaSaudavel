#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

/* =============================================================================
 * TADs (Tipos Abstratos de Dados) DO PROJETO:
 *
 * 1. paciente.h -> TAD Lista Encadeada Simples de Pacientes (Cadastro Geral)
 * 2. fila.h     -> TAD Fila FIFO de Pacientes (Sala de Espera por ordem de chegada)
 * 3. medico.h   -> TAD Lista Circular de Medicos (Escala de plantao / rodizio)
 * 4. pilha.h    -> TAD Pilha LIFO de Atendimentos (Historico para desfazer acoes)
 * ============================================================================= */
#include "paciente.h"
#include "fila.h"
#include "medico.h"
#include "pilha.h"

/* -----------------------------------------------------------------------------
 * FUNCOES AUXILIARES PARA ENTRADA DO USUARIO NO TERMINAL
 *
 * Por que precisamos disso?
 * Lemos uma linha por vez com 'fgets', inclusive para os numeros.
 * Isso evita que um ENTER ou um texto comprido atrapalhe a proxima pergunta.
 * 'strtol' converte a linha em inteiro e permite conferir erros e limites.
 * Se a entrada terminar (EOF), o menu libera a memoria e encerra.
 * ----------------------------------------------------------------------------- */

/* Limpa caracteres restantes no teclado ate encontrar o fim da linha ou EOF */
static void limpar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* Apenas consome o caractere */
    }
}

/* Retorna 1 para uma linha valida e 0 quando a entrada termina. */
static int ler_texto(char *buffer, int tamanho) {
    while (fgets(buffer, tamanho, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[--len] = '\0';
        } else {
            int extra = getchar();
            if (extra != '\n' && extra != EOF) {
                limpar_buffer();
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

/* Zero no EOF nao e uma escolha: o chamador confere feof/ferror antes de agir. */
static int ler_inteiro(const char *mensagem, int minimo, int maximo) {
    char linha[128];
    printf("%s", mensagem);
    while (ler_texto(linha, sizeof linha)) {
        char *fim;
        errno = 0;
        long valor = strtol(linha, &fim, 10);
        if (errno != ERANGE && fim != linha && *fim == '\0' &&
            valor >= minimo && valor <= maximo) {
            return (int)valor;
        }
        printf("Entrada invalida! Digite um inteiro entre %d e %d: ", minimo, maximo);
    }
    return 0;
}

/* -----------------------------------------------------------------------------
 * OPERACAO 1: CADASTRAR NOVO PACIENTE (LISTA ENCADEADA SIMPLES)
 *
 * Conceito:
 * - A lista encadeada aloca cada no dinamicamente na memoria com 'malloc'.
 * - Nao ha limite fixo de pacientes: a memoria cresce conforme a demanda.
 * - Cada no aponta para o proximo paciente ('prox').
 * ----------------------------------------------------------------------------- */
static void acao_cadastrar_paciente(ListaPacientes *cadastro, FilaPacientes *fila) {
    Paciente p = {0};

    printf("\n--- [CADASTRO DE NOVO PACIENTE] ---\n");
    p.id_paciente = ler_inteiro("Digite o ID do paciente (ex: 101): ", 1, INT_MAX);

    printf("Digite o nome completo do paciente: ");
    ler_texto(p.nome, PACIENTE_TAMANHO_NOME);

    p.idade = ler_inteiro("Digite a idade do paciente: ", 0, INT_MAX);

    printf("Digite o sintoma / queixa principal: ");
    ler_texto(p.sintoma, PACIENTE_TAMANHO_SINTOMA);
    if (feof(stdin) || ferror(stdin)) {
        return; /* Cadastro interrompido: nao inserir dados incompletos. */
    }

    /* Inserimos na Lista Encadeada de Pacientes */
    ResultadoPaciente res = inserir_paciente(cadastro, p);

    if (res == PACIENTE_OK) {
        printf("\n[SUCESSO] Paciente '%s' cadastrado no sistema!\n", p.nome);

        /* Pergunta conveniente: ja deseja encaminhar para a sala de espera? */
        int enviar = ler_inteiro("Deseja colocar este paciente na Fila de Espera agora? (1 - Sim / 0 - Nao): ", 0, 1);
        if (enviar == 1) {
            ResultadoFila res_fila = enfileirar_paciente(fila, p);
            if (res_fila == FILA_OK) {
                printf("[OK] Paciente colocado na Sala de Espera (Fila FIFO)!\n");
            } else if (res_fila == FILA_SEM_MEMORIA) {
                printf("[ERRO] Memoria insuficiente para colocar na fila.\n");
            }
        }
    } else if (res == PACIENTE_ID_DUPLICADO) {
        printf("\n[ERRO] Ja existe um paciente cadastrado com o ID %d!\n", p.id_paciente);
    } else if (res == PACIENTE_SEM_MEMORIA) {
        printf("\n[ERRO] O computador ficou sem memoria para alocar o paciente!\n");
    } else {
        printf("\n[ERRO] Dados invalidos fornecidos.\n");
    }
}

/* -----------------------------------------------------------------------------
 * OPERACAO 2: ENFILEIRAR PACIENTE EXISTENTE (FILA FIFO - FIRST IN, FIRST OUT)
 *
 * Conceito:
 * - A Fila organiza os pacientes por ordem de chegada.
 * - O primeiro paciente que entra ('inicio') sera o primeiro a ser atendido.
 * - Novos pacientes entram no final ('fim').
 * ----------------------------------------------------------------------------- */
static void acao_enfileirar_paciente(const ListaPacientes *cadastro, FilaPacientes *fila) {
    printf("\n--- [ENCAMINHAR PACIENTE PARA SALA DE ESPERA] ---\n");
    int id = ler_inteiro("Digite o ID do paciente ja cadastrado: ", 1, INT_MAX);
    if (feof(stdin) || ferror(stdin)) {
        return;
    }

    Paciente p;
    /* Busca o paciente pelo ID na Lista Geral de Pacientes */
    ResultadoPaciente res_busca = buscar_paciente_por_id(cadastro, id, &p);

    if (res_busca != PACIENTE_OK) {
        printf("\n[AVISO] Paciente com ID %d nao encontrado no cadastro!\n", id);
        printf("Cadastre-o primeiro no menu (Opcao 1).\n");
        return;
    }

    /* Insere o paciente na Fila de Espera (ENQUEUE) */
    ResultadoFila res_fila = enfileirar_paciente(fila, p);

    if (res_fila == FILA_OK) {
        printf("\n[SUCESSO] Paciente '%s' (ID %d) entrou na Sala de Espera!\n", p.nome, p.id_paciente);
    } else if (res_fila == FILA_SEM_MEMORIA) {
        printf("\n[ERRO] Falha de memoria ao enfileirar paciente.\n");
    }
}

/* -----------------------------------------------------------------------------
 * OPERACAO 3: CADASTRAR MEDICO NO PLANTAO (LISTA ENCADEADA CIRCULAR)
 *
 * Conceito:
 * - Na lista circular, o ultimo medico aponta de volta para o primeiro.
 * - Nao existe 'fim' com NULL. A lista forma um anel de rodizio.
 * - O ponteiro 'atual' aponta para o medico que esta atendendo no momento.
 * ----------------------------------------------------------------------------- */
static void acao_cadastrar_medico(ListaMedicos *escala) {
    Medico m = {0};

    printf("\n--- [CADASTRAR MEDICO NA ESCALA DE PLANTAO] ---\n");
    m.id_medico = ler_inteiro("Digite o CRM / ID do medico: ", 1, INT_MAX);

    printf("Digite o nome do medico (ex: Dr. Carlos Silva): ");
    ler_texto(m.nome, MEDICO_TAMANHO_NOME);
    if (feof(stdin) || ferror(stdin)) {
        return;
    }

    /* Insere o medico no anel circular */
    ResultadoMedico res = inserir_medico(escala, m);

    if (res == MEDICO_OK) {
        printf("\n[SUCESSO] Medico '%s' inserido na escala de plantao circular!\n", m.nome);
    } else if (res == MEDICO_ID_DUPLICADO) {
        printf("\n[ERRO] Ja existe um medico cadastrado com o ID %d!\n", m.id_medico);
    } else if (res == MEDICO_SEM_MEMORIA) {
        printf("\n[ERRO] Memoria insuficiente para cadastrar medico.\n");
    } else {
        printf("\n[ERRO] Nome invalido ou argumento nulo.\n");
    }
}

/* -----------------------------------------------------------------------------
 * OPERACAO 4: REALIZAR ATENDIMENTO (INTEGRACAO DAS 4 ESTRUTURAS!)
 *
 * Passo a passo didatico:
 * 1. Consulta quem e o medico atual de plantao (Lista Circular).
 * 2. Consulta quem e o proximo paciente na fila sem remove-lo ainda (Fila PEEK).
 * 3. Monta o registro 'Atendimento' e empilha no historico (Pilha PUSH).
 *    -> Se faltar memoria na pilha, abortamos sem perder o paciente da fila!
 * 4. Retira o paciente da sala de espera (Fila DEQUEUE).
 * 5. Rotaciona a escala para o proximo medico atender na rodada seguinte!
 * ----------------------------------------------------------------------------- */
static void acao_realizar_atendimento(ListaMedicos *escala, FilaPacientes *fila, PilhaAtendimentos *historico) {
    printf("\n--- [CHAMADA DE ATENDIMENTO MEDICO] ---\n");

    /* 1. Verifica se ha medico disponivel na escala circular */
    Medico medico_atual;
    ResultadoMedico res_med = consultar_medico_atual(escala, &medico_atual);
    if (res_med == MEDICO_LISTA_VAZIA) {
        printf("\n[BLOQUEIO] Nenhum medico de plantao na escala!\n");
        printf("Cadastre ao menos um medico (Opcao 3) antes de iniciar atendimentos.\n");
        return;
    }

    /* 2. Verifica se ha paciente aguardando na fila FIFO */
    Paciente paciente_vez;
    ResultadoFila res_fila = consultar_frente_fila(fila, &paciente_vez);
    if (res_fila == FILA_VAZIA) {
        printf("\n[BLOQUEIO] A Sala de Espera esta vazia! Nenhum paciente na fila.\n");
        return;
    }

    /* 3. Prepara a 'fotografia' do atendimento com copia dos valores */
    Atendimento reg;
    reg.paciente = paciente_vez;
    reg.medico = medico_atual;

    /* 4. Tenta empilhar o atendimento no historico (LIFO - PUSH) */
    ResultadoPilha res_pilha = empilhar_atendimento(historico, reg);
    if (res_pilha != PILHA_OK) {
        printf("\n[ERRO CRITICO] Falha ao alocar memoria para o historico da pilha.\n");
        printf("O atendimento nao foi realizado para preservar a consistencia da fila.\n");
        return;
    }

    /* 5. Com o registro seguro no historico, remove o paciente da fila (DEQUEUE) */
    Paciente atendido;
    desenfileirar_paciente(fila, &atendido);

    /* 6. Avanca a escala circular para o proximo plantonista (Round-Robin) */
    avancar_medico(escala);

    /* 7. Exibe relatorio didatico da operacao realizada */
    printf("\n=======================================================\n");
    printf("        >>> ATENDIMENTO CONCLUIDO COM SUCESSO! <<<\n");
    printf(" Paciente: %s (ID %d, %d anos)\n", atendido.nome, atendido.id_paciente, atendido.idade);
    printf(" Queixa:   %s\n", atendido.sintoma);
    printf(" Medico:   %s (ID %d)\n", medico_atual.nome, medico_atual.id_medico);
    printf("-------------------------------------------------------\n");
    printf(" * Paciente removido da Sala de Espera (Fila FIFO).\n");
    printf(" * Atendimento registrado no topo do Historico (Pilha LIFO).\n");
    printf(" * Plantao rotacionado para o proximo medico da escala circular.\n");
    printf("=======================================================\n");
}

/* -----------------------------------------------------------------------------
 * OPERACAO 5: DESFAZER ULTIMO ATENDIMENTO (PILHA LIFO - LAST IN, FIRST OUT)
 *
 * Conceito:
 * - A pilha funciona como um "Ctrl+Z".
 * - O ultimo atendimento registrado esta no topo da pilha.
 *
 * Passo a passo seguro:
 * 1. PEEK: Espia o topo da pilha sem remover para obter os dados.
 * 2. Devolve o paciente para a FRENTE da fila de espera ('devolver_inicio_fila').
 * 3. Somente apos a devolucao ser bem-sucedida, removemos o registro da pilha (POP).
 * ----------------------------------------------------------------------------- */
static void acao_desfazer_atendimento(PilhaAtendimentos *historico, FilaPacientes *fila) {
    printf("\n--- [DESFAZER ULTIMO ATENDIMENTO (CTRL+Z)] ---\n");

    /* 1. Consulta o atendimento mais recente no topo da pilha */
    Atendimento ultimo;
    ResultadoPilha res_peek = consultar_topo(historico, &ultimo);

    if (res_peek == PILHA_VAZIA) {
        printf("\n[AVISO] Nenhum atendimento registrado no historico para desfazer!\n");
        return;
    }

    /* 2. Reconstrói o paciente a partir da fotografia do atendimento */
    Paciente p = ultimo.paciente;

    /* 3. Devolve o paciente ao inicio da fila para nao perder sua vez */
    ResultadoFila res_dev = devolver_inicio_fila(fila, p);
    if (res_dev != FILA_OK) {
        printf("\n[ERRO CRITICO] Nao foi possivel devolver o paciente a fila (falta de memoria).\n");
        printf("A pilha foi mantida intacta para tentar novamente depois.\n");
        return;
    }

    /* 4. Agora sim, remove o atendimento do topo da pilha (POP) */
    Atendimento removido;
    desempilhar_atendimento(historico, &removido);

    printf("\n=======================================================\n");
    printf("      >>> ATENDIMENTO DESFEITO COM SUCESSO! <<<\n");
    printf(" Paciente: %s (ID %d) voltou para a FRENTE da Sala de Espera.\n", p.nome, p.id_paciente);
    printf(" Registro com Dr(a). %s foi retirado do historico.\n", removido.medico.nome);
    printf("=======================================================\n");
}

/* -----------------------------------------------------------------------------
 * OPERACAO 6: AVANCAR PLANTAO MANUALMENTE (RODIZIO DA LISTA CIRCULAR)
 * ----------------------------------------------------------------------------- */
static void acao_avancar_plantao(ListaMedicos *escala) {
    printf("\n--- [RODIZIO MANUAL DA ESCALA DE PLANTAO] ---\n");
    ResultadoMedico res = avancar_medico(escala);

    if (res == MEDICO_LISTA_VAZIA) {
        printf("\n[AVISO] Nao ha medicos cadastrados na escala.\n");
    } else {
        Medico atual;
        consultar_medico_atual(escala, &atual);
        printf("\n[OK] Plantao rotacionado! Medico da vez agora: %s (ID %d)\n", atual.nome, atual.id_medico);
    }
}

/* -----------------------------------------------------------------------------
 * CARREGAR DADOS INICIAIS DE EXEMPLO
 *
 * Facilita testes e demonstracao do projeto sem precisar digitar tudo na mao!
 * ----------------------------------------------------------------------------- */
static void carregar_dados_exemplo(ListaPacientes *cadastro, ListaMedicos *escala, FilaPacientes *fila) {
    /* Cadastra Medicos na lista circular */
    Medico m1 = {1, "Dr. Carlos Eduardo (Clinico Geral)"};
    Medico m2 = {2, "Dra. Beatriz Santos (Pediatra)"};
    Medico m3 = {3, "Dr. Andre Oliveira (Ortopedista)"};
    inserir_medico(escala, m1);
    inserir_medico(escala, m2);
    inserir_medico(escala, m3);

    /* Cadastra Pacientes na lista geral */
    Paciente p1 = {101, "Maria Silva", 28, "Dor aguda no joelho"};
    Paciente p2 = {102, "Joao Souza", 45, "Febre e tosse persistente"};
    Paciente p3 = {103, "Ana Clara", 12, "Reacao alergica na pele"};
    inserir_paciente(cadastro, p1);
    inserir_paciente(cadastro, p2);
    inserir_paciente(cadastro, p3);

    /* Coloca pacientes na fila de espera */
    enfileirar_paciente(fila, p1);
    enfileirar_paciente(fila, p2);
    enfileirar_paciente(fila, p3);

    printf("\n[DADOS DE EXEMPLO CARREGADOS COM SUCESSO!]\n");
    printf("- 3 Medicos inseridos na Escala Circular de Plantao.\n");
    printf("- 3 Pacientes cadastrados no Cadastro Geral.\n");
    printf("- 3 Pacientes adicionados na Sala de Espera (Fila FIFO).\n\n");
}

/* =============================================================================
 * PROGRAMA PRINCIPAL (main)
 * ============================================================================= */
int main(void) {
    /* -------------------------------------------------------------------------
     * INICIALIZACAO DOS TADs:
     *
     * O que acontece aqui?
     * Cada funcao inicializadora apenas define os ponteiros de controle como NULL.
     * Exemplo: 'lista.inicio = NULL', 'pilha.topo = NULL'.
     * Isso garante que a memoria nao aponte para lixo antes de usarmos!
     * ------------------------------------------------------------------------- */
    ListaPacientes cadastro;
    FilaPacientes fila_espera;
    ListaMedicos escala_medicos;
    PilhaAtendimentos historico;

    inicializar_lista_pacientes(&cadastro);
    inicializar_fila(&fila_espera);
    inicializar_medicos(&escala_medicos);
    inicializar_pilha(&historico);

    printf("=====================================================\n");
    printf("   BEM-VINDO AO SISTEMA DA CLINICA VIDA SAUDAVEL   \n");
    printf("=====================================================\n");

    int opcao_exemplo = ler_inteiro("Deseja carregar dados de teste pre-definidos? (1 - Sim / 0 - Nao): ", 0, 1);
    if (opcao_exemplo == 1) {
        carregar_dados_exemplo(&cadastro, &escala_medicos, &fila_espera);
    }

    int opcao = -1;

    /* Loop principal do menu interativo */
    do {
        if (feof(stdin) || ferror(stdin)) {
            break;
        }
        printf("\n=====================================================\n");
        printf("               MENU PRINCIPAL DA CLINICA             \n");
        printf("=====================================================\n");
        printf(" [1]  Cadastrar Paciente\n");
        printf(" [2]  Encaminhar Paciente para a Sala de Espera\n");
        printf(" [3]  Cadastrar Medico na Escala\n");
        printf(" [4]  Chamar e Realizar Atendimento\n");
        printf(" [5]  Desfazer Ultimo Atendimento\n");
        printf(" [6]  Passar Plantao Manualmente\n");
        printf("-----------------------------------------------------\n");
        printf(" [7]  Exibir Sala de Espera\n");
        printf(" [8]  Exibir Escala de Medicos\n");
        printf(" [9]  Exibir Historico de Atendimentos\n");
        printf(" [10] Exibir Todos os Pacientes Cadastrados\n");
        printf("-----------------------------------------------------\n");
        printf(" [0]  Encerrar Programa e Liberar Memoria\n");
        printf("=====================================================\n");

        opcao = ler_inteiro("Escolha uma opcao: ", 0, 10);
        if (feof(stdin) || ferror(stdin)) {
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
                printf("\n--- [SALA DE ESPERA] ---\n");
                listar_fila(&fila_espera, stdout);
                break;

            case 8:
                printf("\n--- [ESCALA DE MEDICOS] ---\n");
                listar_medicos(&escala_medicos, stdout);
                break;

            case 9:
                printf("\n--- [HISTORICO DE ATENDIMENTOS] ---\n");
                listar_atendimentos(&historico, stdout);
                break;

            case 10:
                printf("\n--- [CADASTRO GERAL DE PACIENTES] ---\n");
                listar_pacientes(&cadastro, stdout);
                break;

            case 0:
                printf("\nEncerrando o sistema e liberando memoria...\n");
                break;

            default:
                printf("\n[AVISO] Opcao invalida! Digite um numero de 0 a 10.\n");
                break;
        }

    } while (opcao != 0);

    /* -------------------------------------------------------------------------
     * LIBERACAO TOTAL DA MEMORIA (LIMPEZA COM 'free'):
     *
     * Regra de Ouro em C:
     * Para cada 'malloc' feito durante o programa, deve existir um 'free'
     * correspondente.
     * As funcoes abaixo percorrem todos os nos de cada TAD e liberam a memoria,
     * garantindo 0%% de vazamento de memoria (zero memory leak)!
     * ------------------------------------------------------------------------- */
    liberar_fila(&fila_espera);
    liberar_pilha(&historico);
    liberar_medicos(&escala_medicos);
    liberar_lista_pacientes(&cadastro);

    printf("Toda a memoria alocada foi liberada com sucesso.\n");
    printf("Sistema da Clinica Vida Saudavel encerrado. Ate logo!\n");

    return 0;
}
