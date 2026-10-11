#include <stdio.h>
#include <stdlib.h>

typedef struct{
    int id_paciente;
    char nome[50];
    int idade;
    char sintoma[100];
} Paciente;

typedef struct No{
    Paciente dados;
    struct No* prox; 
} No;

/* - malloc é a função que pede memoria
- sizeof calcula quantos bytes de espaço precisa

O básico pra alocar um nó:
No* novo = (No*) malloc(sizeof(No));

Traduzindo: 
"Sistema operacional, veja quanto espaço um No ocupa (sizeof(No)), 
separe esse pedaço de memória para mim (malloc) e me entregue o
endereço de onde ele está (No* novo)."

- Quando tem a variável normal: usa ponto (paciente.idade = 30;).
- Quando tem um ponteiro (um endereço de memória): usa a flecha (novo->proximo = NULL;).

A flecha significa: "Vá até o endereço guardado em novo e altere o campo proximo dele".

*/

No* criar_no(Paciente p){
    // pede memoria pro sistemas pra um novo Nó
    No* novo = (No*) malloc(sizeof(No));

    // só pra verificar se o computador tem memoria disponível
    if (novo == NULL){
        printf("Erro: Memoria cheia!\n");
        return NULL;
    }

    // coloca os dados do paciente dentro do nó
    novo->dados = p;

    // conecta no proximo nó
    novo->prox = NULL;

    // retorna o endereço do nó
    return novo;
}


int main(){
    // paciente de teste
    Paciente p1 = {1, "Mariaa Silva", 28, "Dor de joelho"};

    // criando nó com esse paciente
    No* primeiro = criar_no(p1);

    if (primeiro != NULL){
        printf("Paciente cadastrado com sucesso!\n");
        printf("ID: %d | Nome: %s | Idade: %d\n", 
            primeiro->dados.id_paciente,
            primeiro->dados.nome,
            primeiro->dados.idade);
    }

    return 0;
}