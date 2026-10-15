#ifndef MEDICO_H
#define MEDICO_H

#include <stdio.h>

#define MEDICO_TAMANHO_NOME 50

typedef struct {
    int id_medico;
    char nome[MEDICO_TAMANHO_NOME];
} Medico;

// TAD da lista circular de medicos
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

void inicializar_medicos(ListaMedicos *lista);
ResultadoMedico inserir_medico(ListaMedicos *lista, Medico medico);
ResultadoMedico avancar_medico(ListaMedicos *lista);
ResultadoMedico consultar_medico_atual(const ListaMedicos *lista, Medico *saida);
ResultadoMedico remover_medico_atual(ListaMedicos *lista);
void listar_medicos(const ListaMedicos *lista, FILE *saida);
void liberar_medicos(ListaMedicos *lista);

#endif
