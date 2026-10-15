#include "clinica.h"

// inicializa os 4 tads
void inicializar_clinica(Clinica *clinica) {
    if (clinica != NULL) {
        inicializar_lista_pacientes(&clinica->pacientes);
        inicializar_fila(&clinica->fila);
        inicializar_medicos(&clinica->medicos);
        inicializar_pilha(&clinica->historico);
    }
}

// libera a memoria de todos os tads
void liberar_clinica(Clinica *clinica) {
    if (clinica != NULL) {
        liberar_pilha(&clinica->historico);
        liberar_fila(&clinica->fila);
        liberar_medicos(&clinica->medicos);
        liberar_lista_pacientes(&clinica->pacientes);
    }
}

// busca paciente no cadastro e poe na fila
ResultadoClinica colocar_paciente_na_fila(Clinica *clinica, int id) {
    if (clinica == NULL) {
        return CLINICA_ARGUMENTO_INVALIDO;
    }

    Paciente paciente;
    ResultadoPaciente busca = buscar_paciente_por_id(&clinica->pacientes, id, &paciente);
    if (busca == PACIENTE_LISTA_VAZIA || busca == PACIENTE_NAO_ENCONTRADO) {
        return CLINICA_PACIENTE_NAO_CADASTRADO;
    }
    if (busca != PACIENTE_OK) {
        return CLINICA_ARGUMENTO_INVALIDO;
    }

    ResultadoFila insercao = enfileirar_paciente(&clinica->fila, paciente);
    if (insercao == FILA_SEM_MEMORIA) {
        return CLINICA_SEM_MEMORIA;
    }

    return (insercao == FILA_OK) ? CLINICA_OK : CLINICA_ARGUMENTO_INVALIDO;
}

// atende quem esta na frente da fila e avanca o medico
ResultadoClinica atender_proximo_paciente(Clinica *clinica, Atendimento *saida) {
    if (clinica == NULL || saida == NULL) {
        return CLINICA_ARGUMENTO_INVALIDO;
    }

    Atendimento registro;
    if (consultar_frente_fila(&clinica->fila, &registro.paciente) != FILA_OK) {
        return CLINICA_FILA_VAZIA;
    }
    if (consultar_medico_atual(&clinica->medicos, &registro.medico) != MEDICO_OK) {
        return CLINICA_SEM_MEDICOS;
    }

    // empilha no historico
    ResultadoPilha insercao = empilhar_atendimento(&clinica->historico, registro);
    if (insercao == PILHA_SEM_MEMORIA) {
        return CLINICA_SEM_MEMORIA;
    }
    if (insercao != PILHA_OK) {
        return CLINICA_ARGUMENTO_INVALIDO;
    }

    // tira da fila
    Paciente retirado;
    if (desenfileirar_paciente(&clinica->fila, &retirado) != FILA_OK) {
        Atendimento descartado;
        (void)desempilhar_atendimento(&clinica->historico, &descartado);
        return CLINICA_ESTADO_INVALIDO;
    }

    // roda a escala
    (void)avancar_medico(&clinica->medicos);
    *saida = registro;
    return CLINICA_OK;
}

// desfaz o atendimento: devolve paciente pra fila e desempilha
ResultadoClinica desfazer_ultimo_atendimento(Clinica *clinica, Atendimento *saida) {
    if (clinica == NULL || saida == NULL) {
        return CLINICA_ARGUMENTO_INVALIDO;
    }

    Atendimento registro;
    if (consultar_topo(&clinica->historico, &registro) != PILHA_OK) {
        return CLINICA_HISTORICO_VAZIO;
    }

    // devolve pro inicio da fila
    ResultadoFila devolucao = devolver_inicio_fila(&clinica->fila, registro.paciente);
    if (devolucao == FILA_SEM_MEMORIA) {
        return CLINICA_SEM_MEMORIA;
    }
    if (devolucao != FILA_OK) {
        return CLINICA_ARGUMENTO_INVALIDO;
    }

    // remove da pilha
    if (desempilhar_atendimento(&clinica->historico, &registro) != PILHA_OK) {
        Paciente descartado;
        (void)desenfileirar_paciente(&clinica->fila, &descartado);
        return CLINICA_ESTADO_INVALIDO;
    }

    *saida = registro;
    return CLINICA_OK;
}
