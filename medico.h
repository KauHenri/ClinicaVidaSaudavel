#ifndef MEDICO_H
#define MEDICO_H

#include <stdio.h>

#define MEDICO_TAMANHO_NOME 50

typedef struct {
    int id_medico;
    char nome[MEDICO_TAMANHO_NOME];
} Medico;

/* O formato do no fica privado em medico.c. O TAD guarda apenas o atual. */
typedef struct NoMedico NoMedico;
typedef struct {
    NoMedico *atual;
} ListaMedicos;

typedef enum {
    MEDICO_OK,
    MEDICO_LISTA_VAZIA,
    MEDICO_ID_DUPLICADO,
    MEDICO_SEM_MEMORIA,
    MEDICO_ARGUMENTO_INVALIDO
} ResultadoMedico;

/* Inicialize antes de usar. Nao reinicialize uma lista ainda ocupada. */
void inicializar_medicos(ListaMedicos *lista);

/* Insere ao fim da escala a partir do atual, sem mudar o plantonista.
 * O nome precisa conter '\0' nos 50 bytes. Falhas preservam a lista. */
ResultadoMedico inserir_medico(ListaMedicos *lista, Medico medico);

/* Avanca um passo na escala; com um medico, ele continua atual. */
ResultadoMedico avancar_medico(ListaMedicos *lista);

/* Copia o medico atual sem remover. Em caso de falha, nao altera a saida. */
ResultadoMedico consultar_medico_atual(const ListaMedicos *lista, Medico *saida);

/* Remove o atual e passa o plantao ao seguinte, se existir. */
ResultadoMedico remover_medico_atual(ListaMedicos *lista);

/* Percorre exatamente uma volta, marcando o atual; use stdout no menu.
 * Argumentos NULL nao produzem saida. A consulta nao altera a escala. */
void listar_medicos(const ListaMedicos *lista, FILE *saida);

/* Libera todos os nos e deixa atual == NULL. Pode ser chamada novamente. */
void liberar_medicos(ListaMedicos *lista);

#endif
