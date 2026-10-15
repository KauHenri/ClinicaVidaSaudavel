#include "fila.h"
#include <stdlib.h>

// estrutura interna do no da fila
struct NoFila {
    Paciente dados;
    struct NoFila *prox;
};

// inicia fila vazia
void inicializar_fila(FilaPacientes *fila) {
    if (fila != NULL) {
        fila->inicio = NULL;
        fila->fim = NULL;
    }
}

// insere paciente no fim da fila
ResultadoFila enfileirar_paciente(FilaPacientes *fila, Paciente p) {
    if (fila == NULL) {
        return FILA_ARGUMENTO_INVALIDO;
    }

    // aloca memoria pro novo no
    NoFila *novo = (NoFila*) malloc(sizeof(NoFila));
    if (novo == NULL) {
        return FILA_SEM_MEMORIA;
    }

    novo->dados = p;
    novo->prox = NULL;

    // se a fila estava vazia
    if (fila->inicio == NULL) {
        fila->inicio = novo;
        fila->fim = novo;
    } else {
        fila->fim->prox = novo;
        fila->fim = novo;
    }

    return FILA_OK;
}

// remove o primeiro paciente da fila para atendimento
ResultadoFila desenfileirar_paciente(FilaPacientes *fila, Paciente *saida) {
    if (fila == NULL || saida == NULL) {
        return FILA_ARGUMENTO_INVALIDO;
    }

    if (fila->inicio == NULL) {
        return FILA_VAZIA;
    }

    NoFila *removido = fila->inicio;
    *saida = removido->dados;

    fila->inicio = removido->prox;

    // se esvaziou a fila
    if (fila->inicio == NULL) {
        fila->fim = NULL;
    }

    free(removido);
    return FILA_OK;
}

// olha quem e o primeiro da fila sem remover
ResultadoFila consultar_frente_fila(const FilaPacientes *fila, Paciente *saida) {
    if (fila == NULL || saida == NULL) {
        return FILA_ARGUMENTO_INVALIDO;
    }

    if (fila->inicio == NULL) {
        return FILA_VAZIA;
    }

    *saida = fila->inicio->dados;
    return FILA_OK;
}

// coloca o paciente de volta no inicio da fila (para desfazer)
ResultadoFila devolver_inicio_fila(FilaPacientes *fila, Paciente p) {
    if (fila == NULL) {
        return FILA_ARGUMENTO_INVALIDO;
    }

    NoFila *novo = (NoFila*) malloc(sizeof(NoFila));
    if (novo == NULL) {
        return FILA_SEM_MEMORIA;
    }

    novo->dados = p;
    novo->prox = fila->inicio;
    fila->inicio = novo;

    // se a fila estava vazia
    if (fila->fim == NULL) {
        fila->fim = novo;
    }

    return FILA_OK;
}

// mostra todos os pacientes na fila
void listar_fila(const FilaPacientes *fila, FILE *saida) {
    if (fila == NULL || saida == NULL) {
        return;
    }

    if (fila->inicio == NULL) {
        fprintf(saida, "Fila de espera vazia.\n");
        return;
    }

    fprintf(saida, "\n--- Sala de Espera ---\n");
    NoFila *atual = fila->inicio;
    int pos = 1;

    while (atual != NULL) {
        fprintf(saida, "%d. ID: %d | Nome: %s | Sintoma: %s\n",
                pos,
                atual->dados.id_paciente,
                atual->dados.nome,
                atual->dados.sintoma);
        atual = atual->prox;
        pos++;
    }
}

// libera memoria da fila
void liberar_fila(FilaPacientes *fila) {
    if (fila == NULL) {
        return;
    }

    NoFila *atual = fila->inicio;
    while (atual != NULL) {
        NoFila *prox = atual->prox;
        free(atual);
        atual = prox;
    }

    fila->inicio = NULL;
    fila->fim = NULL;
}
