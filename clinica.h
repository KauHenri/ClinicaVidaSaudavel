#ifndef CLINICA_H
#define CLINICA_H

#include "fila.h"
#include "medico.h"
#include "pilha.h"

// agrupa os quatro TADs do sistema
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

void inicializar_clinica(Clinica *clinica);
void liberar_clinica(Clinica *clinica);
ResultadoClinica colocar_paciente_na_fila(Clinica *clinica, int id);
ResultadoClinica atender_proximo_paciente(Clinica *clinica, Atendimento *saida);
ResultadoClinica desfazer_ultimo_atendimento(Clinica *clinica, Atendimento *saida);

#endif
