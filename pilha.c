#include "pilha.h"

#include <stdlib.h>
#include <string.h>

/* O topo aponta para o atendimento mais recente. Cada no guarda seus
 * dados e aponta para o atendimento anterior; o mais antigo aponta a NULL.
 * As copias continuam validas mesmo se o paciente ou medico for removido. */
struct NoAtendimento {
    Atendimento dados;
    struct NoAtendimento *prox;
};

void inicializar_pilha(PilhaAtendimentos *pilha)
{
    if (pilha != NULL) {
        pilha->topo = NULL;
    }
}

ResultadoPilha empilhar_atendimento(PilhaAtendimentos *pilha, Atendimento dados)
{
    NoAtendimento *novo;

    if (pilha == NULL ||
        memchr(dados.nome_paciente, '\0', sizeof dados.nome_paciente) == NULL ||
        memchr(dados.sintoma, '\0', sizeof dados.sintoma) == NULL ||
        memchr(dados.medico.nome, '\0', sizeof dados.medico.nome) == NULL) {
        return PILHA_ARGUMENTO_INVALIDO;
    }

    novo = malloc(sizeof *novo);
    if (novo == NULL) {
        return PILHA_SEM_MEMORIA;
    }

    /* PUSH: primeiro conectamos o novo no ao antigo topo; so depois
     * atualizamos o topo. Mesmo na primeira insercao, a regra e a mesma,
     * pois o topo anterior vale NULL. Nenhum atendimento e perdido. */
    novo->dados = dados;
    novo->prox = pilha->topo;
    pilha->topo = novo;
    return PILHA_OK;
}

ResultadoPilha desempilhar_atendimento(PilhaAtendimentos *pilha, Atendimento *saida)
{
    NoAtendimento *removido;

    if (pilha == NULL || saida == NULL) {
        return PILHA_ARGUMENTO_INVALIDO;
    }
    if (pilha->topo == NULL) {
        return PILHA_VAZIA;
    }

    /* POP: o ultimo no inserido esta no topo e sai primeiro (LIFO).
     * Copiamos os dados antes do free e promovemos o no anterior a topo.
     * O chamador recebe uma copia independente, suficiente para restaurar
     * o paciente na fila durante o desfazimento. O no removido ja nao e
     * necessario e deve ser liberado para nao vazar memoria. */
    removido = pilha->topo;
    *saida = removido->dados;
    pilha->topo = removido->prox;
    free(removido);
    return PILHA_OK;
}

ResultadoPilha consultar_topo(const PilhaAtendimentos *pilha, Atendimento *saida)
{
    if (pilha == NULL || saida == NULL) {
        return PILHA_ARGUMENTO_INVALIDO;
    }
    if (pilha->topo == NULL) {
        return PILHA_VAZIA;
    }
    *saida = pilha->topo->dados;
    return PILHA_OK;
}

void listar_atendimentos(const PilhaAtendimentos *pilha, FILE *saida)
{
    const NoAtendimento *cursor;

    if (pilha == NULL || saida == NULL) {
        return;
    }
    if (pilha->topo == NULL) {
        fprintf(saida, "Nenhum atendimento registrado.\n");
        return;
    }

    /* A consulta usa outro ponteiro. Alterar o topo aqui perderia o
     * historico. Como os nos apontam aos anteriores, o percurso ja segue
     * a ordem do mais recente ao mais antigo e termina em NULL. */
    for (cursor = pilha->topo; cursor != NULL; cursor = cursor->prox) {
        fprintf(saida, "%sPaciente: %d | Nome: %s | Idade: %d | "
                "Sintoma: %s | Medico: %d | Nome: %s\n",
                cursor == pilha->topo ? "[TOPO] " : "",
                cursor->dados.id_paciente, cursor->dados.nome_paciente,
                cursor->dados.idade_paciente, cursor->dados.sintoma,
                cursor->dados.medico.id_medico, cursor->dados.medico.nome);
    }
}

void liberar_pilha(PilhaAtendimentos *pilha)
{
    if (pilha == NULL) {
        return;
    }

    /* Guardamos o no a liberar e avancamos o topo antes do free.
     * Ao terminar, topo vale NULL; nao acessamos memoria ja liberada. */
    while (pilha->topo != NULL) {
        NoAtendimento *removido = pilha->topo;
        pilha->topo = removido->prox;
        free(removido);
    }
}
