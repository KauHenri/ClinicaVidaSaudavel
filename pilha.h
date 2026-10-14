#ifndef PILHA_H
#define PILHA_H

#include "medico.h"

#define ATENDIMENTO_TAMANHO_NOME_PACIENTE 50
#define ATENDIMENTO_TAMANHO_SINTOMA 100

/* Fotografia do atendimento: guarda valores, sem apontar para outros TADs.
 * Os campos do paciente correspondem aos existentes em main.c. Nao define
 * outro tipo Paciente: a conversao sera feita na integracao conjunta. */
typedef struct {
    int id_paciente;
    char nome_paciente[ATENDIMENTO_TAMANHO_NOME_PACIENTE];
    int idade_paciente;
    char sintoma[ATENDIMENTO_TAMANHO_SINTOMA];
    Medico medico;
} Atendimento;

typedef struct NoAtendimento NoAtendimento;
typedef struct {
    NoAtendimento *topo;
} PilhaAtendimentos;

typedef enum {
    PILHA_OK,
    PILHA_VAZIA,
    PILHA_SEM_MEMORIA,
    PILHA_ARGUMENTO_INVALIDO
} ResultadoPilha;

/* Inicialize antes de usar. Nao reinicialize uma pilha ainda ocupada. */
void inicializar_pilha(PilhaAtendimentos *pilha);

/* PUSH: copia o atendimento no topo. As tres strings devem conter '\0'
 * dentro dos respectivos arrays. Falhas deixam a pilha intacta. */
ResultadoPilha empilhar_atendimento(PilhaAtendimentos *pilha, Atendimento dados);

/* POP: copia o ultimo atendimento para saida e libera seu no.
 * A saida e obrigatoria; falhas preservam a pilha e o valor de saida. */
ResultadoPilha desempilhar_atendimento(PilhaAtendimentos *pilha, Atendimento *saida);

/* PEEK: copia o topo sem remove-lo. Falhas nao alteram a saida. */
ResultadoPilha consultar_topo(const PilhaAtendimentos *pilha, Atendimento *saida);

/* Lista do mais recente ao mais antigo, marcando o topo; use stdout no menu.
 * Argumentos NULL nao produzem saida. Nao modifica a pilha. */
void listar_atendimentos(const PilhaAtendimentos *pilha, FILE *saida);

/* Libera todos os nos e deixa topo == NULL. Pode ser chamada novamente. */
void liberar_pilha(PilhaAtendimentos *pilha);

#endif
