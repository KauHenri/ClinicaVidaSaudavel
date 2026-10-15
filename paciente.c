#include "paciente.h"
#include <stdlib.h>

// estrutura interna do no
struct NoPaciente {
    Paciente dados;
    struct NoPaciente *prox;
};

// inicia a lista vazia
void inicializar_lista_pacientes(ListaPacientes *lista) {
    if (lista != NULL) {
        lista->inicio = NULL;
    }
}

// insere um novo paciente no final da lista
ResultadoPaciente inserir_paciente(ListaPacientes *lista, Paciente p) {
    if (lista == NULL) {
        return PACIENTE_ARGUMENTO_INVALIDO;
    }

    // verifica se o id ja existe
    NoPaciente *atual = lista->inicio;
    while (atual != NULL) {
        if (atual->dados.id_paciente == p.id_paciente) {
            return PACIENTE_ID_DUPLICADO;
        }
        atual = atual->prox;
    }

    // aloca memoria pro novo no
    NoPaciente *novo = (NoPaciente*) malloc(sizeof(NoPaciente));
    if (novo == NULL) {
        return PACIENTE_SEM_MEMORIA;
    }

    novo->dados = p;
    novo->prox = NULL;

    // se a lista estiver vazia, entra no inicio
    if (lista->inicio == NULL) {
        lista->inicio = novo;
    } else {
        // senao caminha ate o ultimo
        NoPaciente *ultimo = lista->inicio;
        while (ultimo->prox != NULL) {
            ultimo = ultimo->prox;
        }
        ultimo->prox = novo;
    }

    return PACIENTE_OK;
}

// procura paciente pelo id
ResultadoPaciente buscar_paciente_por_id(const ListaPacientes *lista, int id, Paciente *saida) {
    if (lista == NULL || saida == NULL) {
        return PACIENTE_ARGUMENTO_INVALIDO;
    }

    if (lista->inicio == NULL) {
        return PACIENTE_LISTA_VAZIA;
    }

    NoPaciente *atual = lista->inicio;
    while (atual != NULL) {
        if (atual->dados.id_paciente == id) {
            *saida = atual->dados;
            return PACIENTE_OK;
        }
        atual = atual->prox;
    }

    return PACIENTE_NAO_ENCONTRADO;
}

// remove paciente da lista pelo id
ResultadoPaciente remover_paciente(ListaPacientes *lista, int id) {
    if (lista == NULL) {
        return PACIENTE_ARGUMENTO_INVALIDO;
    }

    if (lista->inicio == NULL) {
        return PACIENTE_LISTA_VAZIA;
    }

    NoPaciente *atual = lista->inicio;
    NoPaciente *anterior = NULL;

    // procura o paciente guardando o anterior
    while (atual != NULL && atual->dados.id_paciente != id) {
        anterior = atual;
        atual = atual->prox;
    }

    if (atual == NULL) {
        return PACIENTE_NAO_ENCONTRADO;
    }

    // se for o primeiro no
    if (anterior == NULL) {
        lista->inicio = atual->prox;
    } else {
        anterior->prox = atual->prox;
    }

    free(atual);
    return PACIENTE_OK;
}

// mostra todos os pacientes cadastrados
void listar_pacientes(const ListaPacientes *lista, FILE *saida) {
    if (lista == NULL || saida == NULL) {
        return;
    }

    if (lista->inicio == NULL) {
        fprintf(saida, "Nenhum paciente cadastrado.\n");
        return;
    }

    fprintf(saida, "\n--- Lista de Pacientes ---\n");
    NoPaciente *atual = lista->inicio;
    while (atual != NULL) {
        fprintf(saida, "ID: %d | Nome: %s | Idade: %d | Sintoma: %s\n",
                atual->dados.id_paciente,
                atual->dados.nome,
                atual->dados.idade,
                atual->dados.sintoma);
        atual = atual->prox;
    }
}

// libera toda a memoria da lista
void liberar_lista_pacientes(ListaPacientes *lista) {
    if (lista == NULL) {
        return;
    }

    NoPaciente *atual = lista->inicio;
    while (atual != NULL) {
        NoPaciente *prox = atual->prox;
        free(atual);
        atual = prox;
    }

    lista->inicio = NULL;
}
