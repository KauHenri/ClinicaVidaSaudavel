#ifndef FILA_H
#define FILA_H

#include "paciente.h"
#include <stdio.h>

// estrutura interna privada
typedef struct NoFila NoFila;

// TAD da fila (guarda inicio e fim para ser O(1))
typedef struct {
    NoFila *inicio;
    NoFila *fim;
} FilaPacientes;

typedef enum {
    FILA_OK,
    FILA_VAZIA,
    FILA_SEM_MEMORIA,
    FILA_ARGUMENTO_INVALIDO
} ResultadoFila;

void inicializar_fila(FilaPacientes *fila);
ResultadoFila enfileirar_paciente(FilaPacientes *fila, Paciente p);
ResultadoFila desenfileirar_paciente(FilaPacientes *fila, Paciente *saida);
ResultadoFila consultar_frente_fila(const FilaPacientes *fila, Paciente *saida);
ResultadoFila devolver_inicio_fila(FilaPacientes *fila, Paciente p);
void listar_fila(const FilaPacientes *fila, FILE *saida);
void liberar_fila(FilaPacientes *fila);

#endif
