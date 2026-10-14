#ifndef CLINICA_H
#define CLINICA_H

#include "fila.h"
#include "medico.h"
#include "pilha.h"

/* Apenas agrupa os quatro TADs. Os nos continuam nos modulos originais. */
typedef struct {
    ListaPacientes pacientes;
    FilaPacientes fila;
    ListaMedicos medicos;
    PilhaAtendimentos historico;
} Clinica;

typedef enum {
    CLINICA_OK,
    CLINICA_FILA_VAZIA,
    CLINICA_SEM_MEDICOS,
    CLINICA_HISTORICO_VAZIO,
    CLINICA_PACIENTE_NAO_CADASTRADO,
    CLINICA_SEM_MEMORIA,
    CLINICA_ARGUMENTO_INVALIDO,
    CLINICA_ESTADO_INVALIDO
} ResultadoClinica;

/* Inicialize antes de usar. Nao copie uma clinica ocupada por atribuicao. */
void inicializar_clinica(Clinica *clinica);
void liberar_clinica(Clinica *clinica);

/* Busca no cadastro e coloca uma copia no fim da fila. Cada entrada
 * representa uma solicitacao; o mesmo paciente pode entrar mais de uma vez. */
ResultadoClinica colocar_paciente_na_fila(Clinica *clinica, int id);

/* Atende a frente, registra na pilha e avanca o medico. A saida e obrigatoria.
 * Falha de alocacao preserva a fila, a escala, o historico e a saida. */
ResultadoClinica atender_proximo_paciente(Clinica *clinica, Atendimento *saida);

/* Devolve o ultimo paciente ao inicio e so entao faz POP. Nao recua a escala.
 * Falha de alocacao preserva todas as estruturas e a saida. */
ResultadoClinica desfazer_ultimo_atendimento(Clinica *clinica, Atendimento *saida);

#endif
