#ifndef FILA_H
#define FILA_H

#include "paciente.h"
#include <stdio.h>

// Encapsulamento: o formato interno do NoFila fica privado em fila.c
typedef struct NoFila NoFila;

// Estrutura da Fila com ponteiro para o inicio (quem sai) e fim (quem entra)
typedef struct {
    NoFila *inicio;
    NoFila *fim;
} FilaPacientes;

// Codigos de retorno para as operacoes da fila
typedef enum {
    FILA_OK,
    FILA_VAZIA,
    FILA_SEM_MEMORIA,
    FILA_ARGUMENTO_INVALIDO
} ResultadoFila;

// 1. Inicializa a fila definindo inicio e fim como NULL
void inicializar_fila(FilaPacientes *fila);

// 2. ENQUEUE: Insere um paciente no final da fila (sala de espera)
ResultadoFila enfileirar_paciente(FilaPacientes *fila, Paciente p);

// 3. DEQUEUE: Remove o paciente da frente da fila para atendimento e copia seus dados
ResultadoFila desenfileirar_paciente(FilaPacientes *fila, Paciente *saida);

// 4. PEEK: Consulta quem e o proximo da fila sem remove-lo
ResultadoFila consultar_frente_fila(const FilaPacientes *fila, Paciente *saida);

// 5. DEVOLVER AO INICIO: Usado para DESFAZER atendimento (o paciente volta para a frente)
ResultadoFila devolver_inicio_fila(FilaPacientes *fila, Paciente p);

// 6. Exibe a ordem dos pacientes na sala de espera
void listar_fila(const FilaPacientes *fila, FILE *saida);

// 7. Libera toda a memoria da fila
void liberar_fila(FilaPacientes *fila);

#endif

