#include "medico.h"

#include <stdlib.h>
#include <string.h>

/* Cada no guarda uma copia do medico e o endereco do seguinte.
 * Na lista circular, o ultimo no aponta de volta ao atual, nunca a NULL.
 * O ponteiro atual indica onde comeca a escala de plantao neste momento. */
struct NoMedico {
    Medico dados;
    struct NoMedico *prox;
};

void inicializar_medicos(ListaMedicos *lista)
{
    if (lista != NULL) {
        lista->atual = NULL;
    }
}

ResultadoMedico inserir_medico(ListaMedicos *lista, Medico medico)
{
    NoMedico *ultimo = NULL;
    NoMedico *novo;

    if (lista == NULL || memchr(medico.nome, '\0', sizeof medico.nome) == NULL) {
        return MEDICO_ARGUMENTO_INVALIDO;
    }

    /* Uma volta inteira permite rejeitar IDs repetidos e encontrar o ultimo
     * no relativo ao atual. Fazer a busca antes de alocar evita trabalho
     * desnecessario quando o cadastro ja existe. */
    if (lista->atual != NULL) {
        NoMedico *cursor = lista->atual;
        do {
            if (cursor->dados.id_medico == medico.id_medico) {
                return MEDICO_ID_DUPLICADO;
            }
            ultimo = cursor;
            cursor = cursor->prox;
        } while (cursor != lista->atual);
    }

    novo = malloc(sizeof *novo);
    if (novo == NULL) {
        return MEDICO_SEM_MEMORIA;
    }
    novo->dados = medico;

    /* Na primeira insercao, o no aponta para si mesmo e se torna atual.
     * Depois, o novo no entra entre o ultimo e o atual. Os dois enlaces
     * mantem o circulo fechado, sem trocar o medico que esta de plantao. */
    if (lista->atual == NULL) {
        novo->prox = novo;
        lista->atual = novo;
    } else {
        novo->prox = lista->atual;
        ultimo->prox = novo;
    }
    return MEDICO_OK;
}

ResultadoMedico avancar_medico(ListaMedicos *lista)
{
    if (lista == NULL) {
        return MEDICO_ARGUMENTO_INVALIDO;
    }
    if (lista->atual == NULL) {
        return MEDICO_LISTA_VAZIA;
    }

    /* Nao e preciso procurar o primeiro: a propria ligacao circular faz
     * o ultimo retornar ao inicio. Com um no, prox aponta para ele mesmo. */
    lista->atual = lista->atual->prox;
    return MEDICO_OK;
}

ResultadoMedico consultar_medico_atual(const ListaMedicos *lista, Medico *saida)
{
    if (lista == NULL || saida == NULL) {
        return MEDICO_ARGUMENTO_INVALIDO;
    }
    if (lista->atual == NULL) {
        return MEDICO_LISTA_VAZIA;
    }
    *saida = lista->atual->dados;
    return MEDICO_OK;
}

ResultadoMedico remover_medico_atual(ListaMedicos *lista)
{
    NoMedico *removido;

    if (lista == NULL) {
        return MEDICO_ARGUMENTO_INVALIDO;
    }
    if (lista->atual == NULL) {
        return MEDICO_LISTA_VAZIA;
    }
    removido = lista->atual;

    /* Se prox aponta para o proprio no, ele e o unico medico. A lista
     * precisa ficar vazia antes de liberar esse endereco. */
    if (removido->prox == removido) {
        lista->atual = NULL;
    } else {
        NoMedico *anterior = removido;
        /* Como a lista e simplesmente encadeada, precisamos encontrar
         * o antecessor para faze-lo saltar o no que sera removido. */
        while (anterior->prox != removido) {
            anterior = anterior->prox;
        }
        anterior->prox = removido->prox;
        lista->atual = removido->prox;
    }
    free(removido);
    return MEDICO_OK;
}

void listar_medicos(const ListaMedicos *lista, FILE *saida)
{
    const NoMedico *cursor;

    if (lista == NULL || saida == NULL) {
        return;
    }
    if (lista->atual == NULL) {
        fprintf(saida, "Nenhum medico de plantao.\n");
        return;
    }

    /* Esperar NULL causaria um loop infinito. Comecamos no atual e
     * paramos quando voltamos a ele, depois de visitar cada no uma vez.
     * O cursor local preserva o ponteiro de plantao durante a consulta. */
    cursor = lista->atual;
    do {
        fprintf(saida, "%sID: %d | Nome: %s\n",
                cursor == lista->atual ? "[ATUAL] " : "",
                cursor->dados.id_medico, cursor->dados.nome);
        cursor = cursor->prox;
    } while (cursor != lista->atual);
}

void liberar_medicos(ListaMedicos *lista)
{
    NoMedico *ultimo;
    NoMedico *cursor;

    if (lista == NULL || lista->atual == NULL) {
        return;
    }

    /* Apenas na destruicao, abrimos o circulo para terminar em NULL.
     * Assim nao precisamos comparar enderecos de nos ja liberados.
     * Nas operacoes normais a lista permanece sempre circular. */
    ultimo = lista->atual;
    while (ultimo->prox != lista->atual) {
        ultimo = ultimo->prox;
    }
    ultimo->prox = NULL;
    cursor = lista->atual;
    lista->atual = NULL;
    while (cursor != NULL) {
        NoMedico *seguinte = cursor->prox;
        /* Guardamos o proximo endereco antes de liberar o no atual. */
        free(cursor);
        cursor = seguinte;
    }
}
