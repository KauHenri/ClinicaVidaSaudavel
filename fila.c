#include "fila.h"
#include <stdlib.h>

// Definicao interna do nó da fila (privado para manter o encapsulamento)
struct NoFila {
    Paciente dados;
    struct NoFila *prox;
};

// 1. Inicializa a fila como vazia (inicio e fim apontam para NULL)
void inicializar_fila(FilaPacientes *fila) {
    if (fila != NULL) {
        fila->inicio = NULL;
        fila->fim = NULL;
    }
}

// 2. ENQUEUE: Insere um paciente no final da fila (sala de espera)
ResultadoFila enfileirar_paciente(FilaPacientes *fila, Paciente p) {
    // valida ponteiro da fila
    if (fila == NULL) {
        return FILA_ARGUMENTO_INVALIDO;
    }

    // aloca memoria para o novo nó da fila
    NoFila *novo = (NoFila*) malloc(sizeof(NoFila));

    // verifica se o sistema conseguiu liberar memoria
    if (novo == NULL) {
        return FILA_SEM_MEMORIA;
    }

    // preenche os dados do paciente
    novo->dados = p;
    novo->prox = NULL; // quem entra no fim da fila nao tem ninguem atras de si

    // se a fila estava vazia, o novo nó e tanto o inicio quanto o fim
    if (fila->inicio == NULL) {
        fila->inicio = novo;
        fila->fim = novo;
    } else {
        // conecta o antigo ultimo nó ao novo nó
        fila->fim->prox = novo;
        // atualiza o ponteiro de fim para ser o novo nó
        fila->fim = novo;
    }

    return FILA_OK;
}

// 3. DEQUEUE: Remove quem esta no inicio da fila para ser atendido
ResultadoFila desenfileirar_paciente(FilaPacientes *fila, Paciente *saida) {
    // valida argumentos
    if (fila == NULL || saida == NULL) {
        return FILA_ARGUMENTO_INVALIDO;
    }

    // se nao ha ninguem esperando
    if (fila->inicio == NULL) {
        return FILA_VAZIA;
    }

    // guarda o nó que esta na frente da fila
    NoFila *removido = fila->inicio;

    // copia os dados do paciente para a variavel de saida
    *saida = removido->dados;

    // avanca o inicio para o proximo paciente
    fila->inicio = removido->prox;

    // se a fila ficou vazia apos remover, o fim tambem deve apontar para NULL
    if (fila->inicio == NULL) {
        fila->fim = NULL;
    }

    // libera a memoria do nó atendido
    free(removido);

    return FILA_OK;
}

// 4. PEEK: Olha quem e o proximo da fila sem remove-lo
ResultadoFila consultar_frente_fila(const FilaPacientes *fila, Paciente *saida) {
    if (fila == NULL || saida == NULL) {
        return FILA_ARGUMENTO_INVALIDO;
    }

    if (fila->inicio == NULL) {
        return FILA_VAZIA;
    }

    // apenas copia os dados de quem esta no inicio
    *saida = fila->inicio->dados;

    return FILA_OK;
}

// 5. DEVOLVER AO INICIO: Usado na operacao de DESFAZER atendimento
ResultadoFila devolver_inicio_fila(FilaPacientes *fila, Paciente p) {
    if (fila == NULL) {
        return FILA_ARGUMENTO_INVALIDO;
    }

    // aloca memoria para o nó que esta voltando
    NoFila *novo = (NoFila*) malloc(sizeof(NoFila));
    if (novo == NULL) {
        return FILA_SEM_MEMORIA;
    }

    novo->dados = p;

    // ele entra na frente de quem ja estava no inicio
    novo->prox = fila->inicio;
    fila->inicio = novo;

    // se a fila estava completamente vazia, ele tambem passa a ser o fim
    if (fila->fim == NULL) {
        fila->fim = novo;
    }

    return FILA_OK;
}

// 6. Imprime a ordem dos pacientes aguardando na sala de espera
void listar_fila(const FilaPacientes *fila, FILE *saida) {
    if (fila == NULL || saida == NULL) {
        return;
    }

    if (fila->inicio == NULL) {
        fprintf(saida, "A sala de espera esta vazia no momento.\n");
        return;
    }

    fprintf(saida, "\n=== SALA DE ESPERA (ORDEM DE CHEGADA) ===\n");
    NoFila *atual = fila->inicio;
    int posicao = 1;

    while (atual != NULL) {
        fprintf(saida, "%dº da fila -> ID: %d | Nome: %s | Sintoma: %s\n",
                posicao,
                atual->dados.id_paciente,
                atual->dados.nome,
                atual->dados.sintoma);

        atual = atual->prox;
        posicao++;
    }
    fprintf(saida, "=========================================\n\n");
}

// 7. Libera todos os nos da fila da memoria
void liberar_fila(FilaPacientes *fila) {
    if (fila == NULL) {
        return;
    }

    NoFila *atual = fila->inicio;

    // percorre liberando nó por nó
    while (atual != NULL) {
        NoFila *proximo = atual->prox;
        free(atual);
        atual = proximo;
    }

    // zera ambos os ponteiros
    fila->inicio = NULL;
    fila->fim = NULL;
}

