#ifndef PACIENTE_H
#define PACIENTE_H

#include <stdio.h>

#define PACIENTE_TAMANHO_NOME 50
#define PACIENTE_TAMANHO_SINTOMA 100

// dados do paciente
typedef struct {
    int id_paciente;
    char nome[PACIENTE_TAMANHO_NOME];
    int idade;
    char sintoma[PACIENTE_TAMANHO_SINTOMA];
} Paciente;

// encapsulamento da lista
typedef struct NoPaciente NoPaciente;

typedef struct {
    NoPaciente *inicio;
} ListaPacientes;

// códigos de retorno para saber se uma operação deu certo ou deu erro
typedef enum {
    PACIENTE_OK,
    PACIENTE_LISTA_VAZIA,
    PACIENTE_NAO_ENCONTRADO,
    PACIENTE_ID_DUPLICADO,
    PACIENTE_SEM_MEMORIA,
    PACIENTE_ARGUMENTO_INVALIDO
} ResultadoPaciente;

// as funções que a lista vai ter:
void inicializar_lista_pacientes(ListaPacientes *lista);
ResultadoPaciente inserir_paciente(ListaPacientes *lista, Paciente p);
ResultadoPaciente buscar_paciente_por_id(const ListaPacientes *lista, int id, Paciente *saida);
ResultadoPaciente remover_paciente(ListaPacientes *lista, int id);
void listar_pacientes(const ListaPacientes *lista, FILE *saida);
void liberar_lista_pacientes(ListaPacientes *lista);






#endif