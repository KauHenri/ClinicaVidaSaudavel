#ifndef PILHA_H
#define PILHA_H

#include "medico.h"
#include "paciente.h"
#include <stdio.h>

// dados do atendimento guardados no historico
typedef struct {
    Paciente paciente;
    Medico medico;
} Atendimento;

// no privado em pilha.c
typedef struct NoAtendimento NoAtendimento;

// TAD da pilha de historico (LIFO)
typedef struct {
    NoAtendimento *topo;
} PilhaAtendimentos;

typedef enum {
    PILHA_OK,
    PILHA_VAZIA,
    PILHA_SEM_MEMORIA,
    PILHA_ARGUMENTO_INVALIDO
} ResultadoPilha;

void inicializar_pilha(PilhaAtendimentos *pilha);
ResultadoPilha empilhar_atendimento(PilhaAtendimentos *pilha, Atendimento dados);
ResultadoPilha desempilhar_atendimento(PilhaAtendimentos *pilha, Atendimento *saida);
ResultadoPilha consultar_topo(const PilhaAtendimentos *pilha, Atendimento *saida);
void listar_atendimentos(const PilhaAtendimentos *pilha, FILE *saida);
void liberar_pilha(PilhaAtendimentos *pilha);

#endif
