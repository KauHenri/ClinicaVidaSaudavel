#include "clinica.h"

void inicializar_clinica(Clinica *clinica)
{
    if (clinica != NULL) {
        inicializar_lista_pacientes(&clinica->pacientes);
        inicializar_fila(&clinica->fila);
        inicializar_medicos(&clinica->medicos);
        inicializar_pilha(&clinica->historico);
    }
}

void liberar_clinica(Clinica *clinica)
{
    if (clinica != NULL) {
        liberar_pilha(&clinica->historico);
        liberar_fila(&clinica->fila);
        liberar_medicos(&clinica->medicos);
        liberar_lista_pacientes(&clinica->pacientes);
    }
}

ResultadoClinica colocar_paciente_na_fila(Clinica *clinica, int id)
{
    Paciente paciente;
    ResultadoPaciente busca;
    ResultadoFila insercao;

    if (clinica == NULL) {
        return CLINICA_ARGUMENTO_INVALIDO;
    }
    busca = buscar_paciente_por_id(&clinica->pacientes, id, &paciente);
    if (busca == PACIENTE_LISTA_VAZIA || busca == PACIENTE_NAO_ENCONTRADO) {
        return CLINICA_PACIENTE_NAO_CADASTRADO;
    }
    if (busca != PACIENTE_OK) {
        return CLINICA_ARGUMENTO_INVALIDO;
    }
    insercao = enfileirar_paciente(&clinica->fila, paciente);
    if (insercao == FILA_SEM_MEMORIA) {
        return CLINICA_SEM_MEMORIA;
    }
    return insercao == FILA_OK ? CLINICA_OK : CLINICA_ARGUMENTO_INVALIDO;
}

ResultadoClinica atender_proximo_paciente(Clinica *clinica, Atendimento *saida)
{
    Atendimento registro;
    Paciente retirado;
    ResultadoPilha insercao;

    if (clinica == NULL || saida == NULL) {
        return CLINICA_ARGUMENTO_INVALIDO;
    }
    if (consultar_frente_fila(&clinica->fila, &registro.paciente) != FILA_OK) {
        return CLINICA_FILA_VAZIA;
    }
    if (consultar_medico_atual(&clinica->medicos, &registro.medico) != MEDICO_OK) {
        return CLINICA_SEM_MEDICOS;
    }

    /* Consultamos sem retirar. PUSH e a unica etapa que aloca memoria aqui.
     * Se ela falhar, o paciente ainda espera na mesma posicao, o medico
     * continua atual e o historico anterior permanece intacto. */
    insercao = empilhar_atendimento(&clinica->historico, registro);
    if (insercao == PILHA_SEM_MEMORIA) {
        return CLINICA_SEM_MEMORIA;
    }
    if (insercao != PILHA_OK) {
        return CLINICA_ARGUMENTO_INVALIDO;
    }

    /* A retirada nao aloca memoria. Em uma clinica inicializada, sem
     * alteracoes entre as chamadas, ela deve remover o paciente consultado.
     * Se houver uma falha inesperada, desfazemos o PUSH antes de retornar. */
    if (desenfileirar_paciente(&clinica->fila, &retirado) != FILA_OK) {
        Atendimento descartado;
        (void)desempilhar_atendimento(&clinica->historico, &descartado);
        return CLINICA_ESTADO_INVALIDO;
    }
    /* A consulta anterior confirmou um medico; o circulo nao mudou desde
     * entao. O avanco nao aloca e so ocorre apos concluir o atendimento. */
    (void)avancar_medico(&clinica->medicos);
    *saida = registro;
    return CLINICA_OK;
}

ResultadoClinica desfazer_ultimo_atendimento(Clinica *clinica, Atendimento *saida)
{
    Atendimento registro;
    ResultadoFila devolucao;

    if (clinica == NULL || saida == NULL) {
        return CLINICA_ARGUMENTO_INVALIDO;
    }
    if (consultar_topo(&clinica->historico, &registro) != PILHA_OK) {
        return CLINICA_HISTORICO_VAZIO;
    }

    /* A devolucao precisa de um novo no na fila. Fazemos PEEK primeiro e
     * mantemos o registro na pilha enquanto essa alocacao nao terminar.
     * Assim, se faltar memoria, podemos tentar desfazer novamente depois. */
    devolucao = devolver_inicio_fila(&clinica->fila, registro.paciente);
    if (devolucao == FILA_SEM_MEMORIA) {
        return CLINICA_SEM_MEMORIA;
    }
    if (devolucao != FILA_OK) {
        return CLINICA_ARGUMENTO_INVALIDO;
    }

    /* POP nao aloca. Se ocorrer uma falha inesperada, retiramos apenas a
     * entrada que acabamos de devolver, preservando a fila anterior. */
    if (desempilhar_atendimento(&clinica->historico, &registro) != PILHA_OK) {
        Paciente descartado;
        (void)desenfileirar_paciente(&clinica->fila, &descartado);
        return CLINICA_ESTADO_INVALIDO;
    }
    *saida = registro;
    return CLINICA_OK;
}
