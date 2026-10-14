#include "paciente.h"
#include <stdlib.h>
#include <string.h>

// aqui revela o que tem dentro do NoPaciente la em paciente.h
struct NoPaciente {
    Paciente dados;
    struct NoPaciente *prox;
};

// 1. Inicializa a lista definindo o inicio como NULL
void inicializar_lista_pacientes(ListaPacientes *lista) {
    if (lista != NULL) {
        // inicia a lista vazia
        lista->inicio = NULL;
    }
}

// 2. Insere um novo paciente no final da lista encadeada
ResultadoPaciente inserir_paciente(ListaPacientes *lista, Paciente p) {
    // valida se o ponteiro da lista e valido
    if (lista == NULL) {
        return PACIENTE_ARGUMENTO_INVALIDO;
    }

    // ponteiro auxiliar que comeca no primeiro paciente para buscar duplicados
    NoPaciente *atual = lista->inicio;

    while (atual != NULL) {
        // compara o ID do paciente atual com o ID do paciente novo que queremos cadastrar
        if (atual->dados.id_paciente == p.id_paciente) {
            return PACIENTE_ID_DUPLICADO; // ja existe esse ID cadastrado
        }

        // da um passo para a frente: pula pro proximo paciente
        atual = atual->prox;
    }

    // aloca memoria para o novo nó do paciente
    NoPaciente *novo = (NoPaciente*) malloc(sizeof(NoPaciente));

    // verifica se o sistema conseguiu liberar memoria
    if (novo == NULL) {
        return PACIENTE_SEM_MEMORIA;
    }

    // preenche os dados do novo nó
    novo->dados = p;
    novo->prox = NULL; // como vai pro final da lista, o proximo dele e NULL

    // se a lista estiver vazia, o novo nó sera o primeiro
    if (lista->inicio == NULL) {
        lista->inicio = novo;
    } else {
        // se ja existirem pacientes, caminha ate o ultimo nó
        NoPaciente *ultimo = lista->inicio;
        while (ultimo->prox != NULL) {
            ultimo = ultimo->prox;
        }

        // conecta o ultimo nó ao novo nó
        ultimo->prox = novo;
    }

    return PACIENTE_OK;
}

// 3. Busca um paciente pelo ID e copia os dados para a variavel de saida
ResultadoPaciente buscar_paciente_por_id(const ListaPacientes *lista, int id, Paciente *saida) {
    // valida se a lista e o ponteiro de saida sao validos
    if (lista == NULL || saida == NULL) {
        return PACIENTE_ARGUMENTO_INVALIDO;
    }

    // se a lista estiver vazia, nao ha o que buscar
    if (lista->inicio == NULL) {
        return PACIENTE_LISTA_VAZIA;
    }

    // ponteiro auxiliar para percorrer a lista
    NoPaciente *atual = lista->inicio;

    while (atual != NULL) {
        // se encontrou o ID procurado
        if (atual->dados.id_paciente == id) {
            *saida = atual->dados; // copia os dados do paciente para a saida
            return PACIENTE_OK;
        }
        atual = atual->prox;
    }

    // percorreu a lista inteira e nao encontrou
    return PACIENTE_NAO_ENCONTRADO;
}

// 4. Remove um paciente da lista pelo ID e libera a memoria dele
ResultadoPaciente remover_paciente(ListaPacientes *lista, int id) {
    // valida ponteiro da lista
    if (lista == NULL) {
        return PACIENTE_ARGUMENTO_INVALIDO;
    }

    // se a lista estiver vazia
    if (lista->inicio == NULL) {
        return PACIENTE_LISTA_VAZIA;
    }

    NoPaciente *atual = lista->inicio;
    NoPaciente *anterior = NULL;

    // procura o paciente pelo ID guardando quem era o anterior
    while (atual != NULL && atual->dados.id_paciente != id) {
        anterior = atual;
        atual = atual->prox;
    }

    // se chegou no final e nao achou
    if (atual == NULL) {
        return PACIENTE_NAO_ENCONTRADO;
    }

    // caso especial: o nó a ser removido e o primeiro da lista
    if (anterior == NULL) {
        lista->inicio = atual->prox; // o segundo nó vira o novo primeiro
    } else {
        // caso geral: pula o nó atual, ligando o anterior direto no proximo
        anterior->prox = atual->prox;
    }

    // libera a memoria do nó removido para nao vazar memoria
    free(atual);

    return PACIENTE_OK;
}

// 5. Exibe todos os pacientes cadastrados na lista
void listar_pacientes(const ListaPacientes *lista, FILE *saida) {
    if (lista == NULL || saida == NULL) {
        return;
    }

    if (lista->inicio == NULL) {
        fprintf(saida, "Nenhum paciente cadastrado no sistema.\n");
        return;
    }

    fprintf(saida, "\n=== LISTA DE PACIENTES CADASTRADOS ===\n");
    NoPaciente *atual = lista->inicio;

    while (atual != NULL) {
        fprintf(saida, "ID: %d | Nome: %s | Idade: %d | Sintoma: %s\n",
                atual->dados.id_paciente,
                atual->dados.nome,
                atual->dados.idade,
                atual->dados.sintoma);

        atual = atual->prox;
    }
    fprintf(saida, "======================================\n\n");
}

// 6. Libera todos os nos da lista e zera o ponteiro inicio
void liberar_lista_pacientes(ListaPacientes *lista) {
    if (lista == NULL) {
        return;
    }

    NoPaciente *atual = lista->inicio;

    // percorre liberando no por no
    while (atual != NULL) {
        NoPaciente *proximo = atual->prox; // guarda o endereco do proximo antes de apagar o atual
        free(atual);                       // libera a memoria do nó atual
        atual = proximo;                   // avanca para o proximo
    }

    // deixa a lista limpa e vazia
    lista->inicio = NULL;
}
