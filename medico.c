#include "medico.h"
#include <stdlib.h>
#include <string.h>

// estrutura interna do no da lista circular
struct NoMedico {
    Medico dados;
    struct NoMedico *prox;
};

// inicia lista de medicos vazia
void inicializar_medicos(ListaMedicos *lista) {
    if (lista != NULL) {
        lista->atual = NULL;
    }
}

// insere novo medico na lista circular
ResultadoMedico inserir_medico(ListaMedicos *lista, Medico medico) {
    if (lista == NULL || memchr(medico.nome, '\0', sizeof(medico.nome)) == NULL) {
        return MEDICO_ARGUMENTO_INVALIDO;
    }

    NoMedico *ultimo = NULL;

    // se ja tem medicos, da uma volta pra ver se tem id duplicado
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

    // aloca memoria pro novo no
    NoMedico *novo = (NoMedico*) malloc(sizeof(NoMedico));
    if (novo == NULL) {
        return MEDICO_SEM_MEMORIA;
    }
    novo->dados = medico;

    // se for o primeiro medico, aponta para ele mesmo
    if (lista->atual == NULL) {
        novo->prox = novo;
        lista->atual = novo;
    } else {
        // insere no anel circular
        novo->prox = lista->atual;
        ultimo->prox = novo;
    }

    return MEDICO_OK;
}

// passa a vez pro proximo medico da escala
ResultadoMedico avancar_medico(ListaMedicos *lista) {
    if (lista == NULL) {
        return MEDICO_ARGUMENTO_INVALIDO;
    }
    if (lista->atual == NULL) {
        return MEDICO_LISTA_VAZIA;
    }

    lista->atual = lista->atual->prox;
    return MEDICO_OK;
}

// pega os dados do medico atual
ResultadoMedico consultar_medico_atual(const ListaMedicos *lista, Medico *saida) {
    if (lista == NULL || saida == NULL) {
        return MEDICO_ARGUMENTO_INVALIDO;
    }
    if (lista->atual == NULL) {
        return MEDICO_LISTA_VAZIA;
    }

    *saida = lista->atual->dados;
    return MEDICO_OK;
}

// remove o medico atual do plantao
ResultadoMedico remover_medico_atual(ListaMedicos *lista) {
    if (lista == NULL) {
        return MEDICO_ARGUMENTO_INVALIDO;
    }
    if (lista->atual == NULL) {
        return MEDICO_LISTA_VAZIA;
    }

    NoMedico *removido = lista->atual;

    // se tiver apenas um medico na lista
    if (removido->prox == removido) {
        lista->atual = NULL;
    } else {
        // acha o anterior para atualizar o ponteiro
        NoMedico *anterior = removido;
        while (anterior->prox != removido) {
            anterior = anterior->prox;
        }
        anterior->prox = removido->prox;
        lista->atual = removido->prox;
    }

    free(removido);
    return MEDICO_OK;
}

// mostra todos os medicos dando uma volta completa
void listar_medicos(const ListaMedicos *lista, FILE *saida) {
    if (lista == NULL || saida == NULL) {
        return;
    }
    if (lista->atual == NULL) {
        fprintf(saida, "Nenhum medico de plantao.\n");
        return;
    }

    const NoMedico *cursor = lista->atual;
    do {
        fprintf(saida, "%sID: %d | Nome: %s\n",
                cursor == lista->atual ? "[ATUAL] " : "",
                cursor->dados.id_medico, cursor->dados.nome);
        cursor = cursor->prox;
    } while (cursor != lista->atual);
}

// libera toda a memoria da lista circular
void liberar_medicos(ListaMedicos *lista) {
    if (lista == NULL || lista->atual == NULL) {
        return;
    }

    // quebra o ciclo para virar uma lista simples terminada em NULL
    NoMedico *ultimo = lista->atual;
    while (ultimo->prox != lista->atual) {
        ultimo = ultimo->prox;
    }
    ultimo->prox = NULL;

    NoMedico *cursor = lista->atual;
    lista->atual = NULL;

    while (cursor != NULL) {
        NoMedico *prox = cursor->prox;
        free(cursor);
        cursor = prox;
    }
}
