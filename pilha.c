#include "pilha.h"
#include <stdlib.h>
#include <string.h>

// estrutura interna do no da pilha
struct NoAtendimento {
    Atendimento dados;
    struct NoAtendimento *prox;
};

// inicia pilha vazia
void inicializar_pilha(PilhaAtendimentos *pilha) {
    if (pilha != NULL) {
        pilha->topo = NULL;
    }
}

// empilha um novo atendimento no topo (push)
ResultadoPilha empilhar_atendimento(PilhaAtendimentos *pilha, Atendimento dados) {
    if (pilha == NULL ||
        memchr(dados.paciente.nome, '\0', sizeof(dados.paciente.nome)) == NULL ||
        memchr(dados.paciente.sintoma, '\0', sizeof(dados.paciente.sintoma)) == NULL ||
        memchr(dados.medico.nome, '\0', sizeof(dados.medico.nome)) == NULL) {
        return PILHA_ARGUMENTO_INVALIDO;
    }

    NoAtendimento *novo = (NoAtendimento*) malloc(sizeof(NoAtendimento));
    if (novo == NULL) {
        return PILHA_SEM_MEMORIA;
    }

    novo->dados = dados;
    novo->prox = pilha->topo;
    pilha->topo = novo;

    return PILHA_OK;
}

// desempilha o atendimento do topo (pop)
ResultadoPilha desempilhar_atendimento(PilhaAtendimentos *pilha, Atendimento *saida) {
    if (pilha == NULL || saida == NULL) {
        return PILHA_ARGUMENTO_INVALIDO;
    }
    if (pilha->topo == NULL) {
        return PILHA_VAZIA;
    }

    NoAtendimento *removido = pilha->topo;
    *saida = removido->dados;
    pilha->topo = removido->prox;

    free(removido);
    return PILHA_OK;
}

// olha o topo da pilha sem remover
ResultadoPilha consultar_topo(const PilhaAtendimentos *pilha, Atendimento *saida) {
    if (pilha == NULL || saida == NULL) {
        return PILHA_ARGUMENTO_INVALIDO;
    }
    if (pilha->topo == NULL) {
        return PILHA_VAZIA;
    }

    *saida = pilha->topo->dados;
    return PILHA_OK;
}

// imprime o historico do mais recente pro mais antigo
void listar_atendimentos(const PilhaAtendimentos *pilha, FILE *saida) {
    if (pilha == NULL || saida == NULL) {
        return;
    }
    if (pilha->topo == NULL) {
        fprintf(saida, "Nenhum atendimento registrado.\n");
        return;
    }

    const NoAtendimento *cursor = pilha->topo;
    while (cursor != NULL) {
        fprintf(saida, "%sPaciente: %d | Nome: %s | Idade: %d | Sintoma: %s | Medico: %d | Nome: %s\n",
                cursor == pilha->topo ? "[TOPO] " : "",
                cursor->dados.paciente.id_paciente, cursor->dados.paciente.nome,
                cursor->dados.paciente.idade, cursor->dados.paciente.sintoma,
                cursor->dados.medico.id_medico, cursor->dados.medico.nome);
        cursor = cursor->prox;
    }
}

// libera toda a memoria da pilha
void liberar_pilha(PilhaAtendimentos *pilha) {
    if (pilha == NULL) {
        return;
    }

    while (pilha->topo != NULL) {
        NoAtendimento *removido = pilha->topo;
        pilha->topo = removido->prox;
        free(removido);
    }
}
