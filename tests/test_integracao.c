#include "clinica.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef TESTAR_ALOCACAO
#include "alocacao_teste.h"
#endif

static Paciente exemplo_paciente(int id)
{
    Paciente paciente = {id, "Maria", 28, "Dor de joelho"};
    return paciente;
}

static void conferir_paciente(Paciente paciente, int id)
{
    assert(paciente.id_paciente == id);
    assert(strcmp(paciente.nome, "Maria") == 0);
    assert(paciente.idade == 28);
    assert(strcmp(paciente.sintoma, "Dor de joelho") == 0);
}

static void conferir_atual(const Clinica *clinica, int id)
{
    Medico medico;
    assert(consultar_medico_atual(&clinica->medicos, &medico) == MEDICO_OK);
    assert(medico.id_medico == id);
}

static void conferir_frente(const Clinica *clinica, int id)
{
    Paciente paciente;
    assert(consultar_frente_fila(&clinica->fila, &paciente) == FILA_OK);
    conferir_paciente(paciente, id);
}

static void testar_cadastro_e_fila(void)
{
    ListaPacientes lista;
    FilaPacientes fila;
    Paciente saida = exemplo_paciente(99);
    Paciente origem = exemplo_paciente(1);

    inicializar_lista_pacientes(&lista);
    inicializar_fila(&fila);
    assert(buscar_paciente_por_id(&lista, 1, &saida) == PACIENTE_LISTA_VAZIA);
    assert(remover_paciente(&lista, 1) == PACIENTE_LISTA_VAZIA);
    assert(desenfileirar_paciente(&fila, &saida) == FILA_VAZIA);
    assert(consultar_frente_fila(&fila, &saida) == FILA_VAZIA);
    conferir_paciente(saida, 99);

    assert(inserir_paciente(&lista, origem) == PACIENTE_OK);
    strcpy(origem.nome, "Alterado");
    assert(inserir_paciente(&lista, exemplo_paciente(2)) == PACIENTE_OK);
    assert(inserir_paciente(&lista, exemplo_paciente(3)) == PACIENTE_OK);
    assert(inserir_paciente(&lista, exemplo_paciente(2)) == PACIENTE_ID_DUPLICADO);
    assert(buscar_paciente_por_id(&lista, 1, &saida) == PACIENTE_OK);
    conferir_paciente(saida, 1);
    assert(buscar_paciente_por_id(&lista, 99, &saida) == PACIENTE_NAO_ENCONTRADO);
    conferir_paciente(saida, 1);
    assert(enfileirar_paciente(&fila, saida) == FILA_OK);
    assert(enfileirar_paciente(&fila, exemplo_paciente(2)) == FILA_OK);
    assert(devolver_inicio_fila(&fila, exemplo_paciente(3)) == FILA_OK);
    assert(remover_paciente(&lista, 2) == PACIENTE_OK);
    assert(remover_paciente(&lista, 1) == PACIENTE_OK);
    assert(remover_paciente(&lista, 3) == PACIENTE_OK);
    assert(lista.inicio == NULL);

    /* A fila tem copias: excluir o cadastro nao apaga quem ja aguardava. */
    assert(desenfileirar_paciente(&fila, &saida) == FILA_OK);
    conferir_paciente(saida, 3);
    assert(desenfileirar_paciente(&fila, &saida) == FILA_OK);
    conferir_paciente(saida, 1);
    assert(desenfileirar_paciente(&fila, &saida) == FILA_OK);
    conferir_paciente(saida, 2);
    assert(fila.inicio == NULL && fila.fim == NULL);
    assert(devolver_inicio_fila(&fila, exemplo_paciente(1)) == FILA_OK);
    assert(fila.inicio == fila.fim);
    assert(desenfileirar_paciente(&fila, &saida) == FILA_OK);
    assert(fila.inicio == NULL && fila.fim == NULL);

    assert(inserir_paciente(NULL, exemplo_paciente(1)) == PACIENTE_ARGUMENTO_INVALIDO);
    assert(buscar_paciente_por_id(&lista, 1, NULL) == PACIENTE_ARGUMENTO_INVALIDO);
    assert(remover_paciente(NULL, 1) == PACIENTE_ARGUMENTO_INVALIDO);
    assert(enfileirar_paciente(NULL, exemplo_paciente(1)) == FILA_ARGUMENTO_INVALIDO);
    assert(consultar_frente_fila(&fila, NULL) == FILA_ARGUMENTO_INVALIDO);
    assert(desenfileirar_paciente(&fila, NULL) == FILA_ARGUMENTO_INVALIDO);
    assert(devolver_inicio_fila(NULL, exemplo_paciente(1)) == FILA_ARGUMENTO_INVALIDO);

    assert(inserir_paciente(&lista, exemplo_paciente(1)) == PACIENTE_OK);
    assert(inserir_paciente(&lista, exemplo_paciente(2)) == PACIENTE_OK);
    assert(enfileirar_paciente(&fila, exemplo_paciente(1)) == FILA_OK);
    assert(enfileirar_paciente(&fila, exemplo_paciente(2)) == FILA_OK);
    liberar_fila(&fila);
    liberar_lista_pacientes(&lista);
    assert(lista.inicio == NULL && fila.inicio == NULL && fila.fim == NULL);
    liberar_fila(&fila);
    liberar_lista_pacientes(&lista);
    puts("OK: cadastro, busca, remocoes, FIFO, devolucao ao inicio e copias.");
}

static void preparar_clinica(Clinica *clinica)
{
    int id;
    inicializar_clinica(clinica);
    for (id = 1; id <= 3; ++id) {
        assert(inserir_paciente(&clinica->pacientes, exemplo_paciente(id)) == PACIENTE_OK);
        assert(colocar_paciente_na_fila(clinica, id) == CLINICA_OK);
    }
    assert(inserir_medico(&clinica->medicos, (Medico){10, "Ana"}) == MEDICO_OK);
    assert(inserir_medico(&clinica->medicos, (Medico){20, "Bruno"}) == MEDICO_OK);
}

static void testar_fluxo_completo(void)
{
    Clinica clinica;
    Atendimento saida = {{99, "Maria", 28, "Dor de joelho"}, {99, "Sentinela"}};
    Atendimento topo;
    Paciente paciente;
    int id;

    inicializar_clinica(&clinica);
    assert(atender_proximo_paciente(&clinica, &saida) == CLINICA_FILA_VAZIA);
    assert(desfazer_ultimo_atendimento(&clinica, &saida) == CLINICA_HISTORICO_VAZIO);
    assert(colocar_paciente_na_fila(&clinica, 1) == CLINICA_PACIENTE_NAO_CADASTRADO);
    conferir_paciente(saida.paciente, 99);
    assert(inserir_paciente(&clinica.pacientes, exemplo_paciente(1)) == PACIENTE_OK);
    assert(colocar_paciente_na_fila(&clinica, 1) == CLINICA_OK);
    assert(atender_proximo_paciente(&clinica, &saida) == CLINICA_SEM_MEDICOS);
    conferir_frente(&clinica, 1);
    assert(clinica.historico.topo == NULL);
    liberar_clinica(&clinica);

    preparar_clinica(&clinica);
    assert(colocar_paciente_na_fila(&clinica, 99) == CLINICA_PACIENTE_NAO_CADASTRADO);
    assert(atender_proximo_paciente(&clinica, &saida) == CLINICA_OK);
    conferir_paciente(saida.paciente, 1);
    assert(saida.medico.id_medico == 10);
    conferir_atual(&clinica, 20);
    conferir_frente(&clinica, 2);
    assert(atender_proximo_paciente(&clinica, &saida) == CLINICA_OK);
    conferir_paciente(saida.paciente, 2);
    assert(saida.medico.id_medico == 20);
    conferir_atual(&clinica, 10);
    conferir_frente(&clinica, 3);

    assert(desfazer_ultimo_atendimento(&clinica, &saida) == CLINICA_OK);
    conferir_paciente(saida.paciente, 2);
    conferir_frente(&clinica, 2);
    conferir_atual(&clinica, 10);
    assert(consultar_topo(&clinica.historico, &topo) == PILHA_OK);
    conferir_paciente(topo.paciente, 1);
    assert(atender_proximo_paciente(&clinica, &saida) == CLINICA_OK);
    conferir_paciente(saida.paciente, 2);
    assert(saida.medico.id_medico == 10);
    conferir_atual(&clinica, 20);

    assert(desfazer_ultimo_atendimento(&clinica, &saida) == CLINICA_OK);
    assert(desfazer_ultimo_atendimento(&clinica, &saida) == CLINICA_OK);
    assert(desfazer_ultimo_atendimento(&clinica, &saida) == CLINICA_HISTORICO_VAZIO);
    for (id = 1; id <= 3; ++id) {
        assert(desenfileirar_paciente(&clinica.fila, &paciente) == FILA_OK);
        conferir_paciente(paciente, id);
    }
    assert(clinica.fila.inicio == NULL && clinica.fila.fim == NULL);
    assert(atender_proximo_paciente(&clinica, &saida) == CLINICA_FILA_VAZIA);

    /* A FIFO permite novas solicitacoes do mesmo paciente, como no contrato
     * original do colega. Duas entradas continuam sendo dois nos distintos. */
    assert(colocar_paciente_na_fila(&clinica, 1) == CLINICA_OK);
    assert(colocar_paciente_na_fila(&clinica, 1) == CLINICA_OK);
    assert(atender_proximo_paciente(&clinica, &saida) == CLINICA_OK);
    assert(remover_paciente(&clinica.pacientes, 1) == PACIENTE_OK);
    assert(remover_medico_atual(&clinica.medicos) == MEDICO_OK);
    assert(remover_medico_atual(&clinica.medicos) == MEDICO_OK);
    assert(desfazer_ultimo_atendimento(&clinica, &saida) == CLINICA_OK);
    conferir_paciente(saida.paciente, 1);
    assert(colocar_paciente_na_fila(&clinica, 1) == CLINICA_PACIENTE_NAO_CADASTRADO);
    assert(desenfileirar_paciente(&clinica.fila, &paciente) == FILA_OK);
    conferir_paciente(paciente, 1);
    assert(desenfileirar_paciente(&clinica.fila, &paciente) == FILA_OK);
    conferir_paciente(paciente, 1);

    assert(atender_proximo_paciente(NULL, &saida) == CLINICA_ARGUMENTO_INVALIDO);
    assert(atender_proximo_paciente(&clinica, NULL) == CLINICA_ARGUMENTO_INVALIDO);
    assert(desfazer_ultimo_atendimento(NULL, &saida) == CLINICA_ARGUMENTO_INVALIDO);
    assert(desfazer_ultimo_atendimento(&clinica, NULL) == CLINICA_ARGUMENTO_INVALIDO);
    assert(colocar_paciente_na_fila(NULL, 1) == CLINICA_ARGUMENTO_INVALIDO);
    liberar_clinica(&clinica);
    assert(clinica.pacientes.inicio == NULL);
    assert(clinica.medicos.atual == NULL && clinica.historico.topo == NULL);
    liberar_clinica(&clinica);
    inicializar_clinica(NULL);
    liberar_clinica(NULL);
    puts("OK: fluxo completo, rotacao medica, LIFO, desfazimentos e historico independente.");
}

#ifdef TESTAR_ALOCACAO
static void testar_falhas_integracao(void)
{
    Clinica clinica;
    Atendimento saida = {{99, "Maria", 28, "Dor de joelho"}, {99, "Sentinela"}};
    Atendimento topo;
    Paciente paciente;
    NoFila *inicio_anterior;
    NoFila *fim_anterior;
    NoAtendimento *topo_anterior;

    inicializar_clinica(&clinica);
    falhar_proxima_alocacao();
    assert(inserir_paciente(&clinica.pacientes, exemplo_paciente(1)) == PACIENTE_SEM_MEMORIA);
    assert(clinica.pacientes.inicio == NULL);
    assert(inserir_paciente(&clinica.pacientes, exemplo_paciente(1)) == PACIENTE_OK);
    falhar_proxima_alocacao();
    assert(inserir_paciente(&clinica.pacientes, exemplo_paciente(2)) == PACIENTE_SEM_MEMORIA);
    assert(buscar_paciente_por_id(&clinica.pacientes, 1, &paciente) == PACIENTE_OK);
    conferir_paciente(paciente, 1);
    falhar_proxima_alocacao();
    assert(colocar_paciente_na_fila(&clinica, 1) == CLINICA_SEM_MEMORIA);
    assert(clinica.fila.inicio == NULL && clinica.fila.fim == NULL);
    falhar_proxima_alocacao();
    assert(devolver_inicio_fila(&clinica.fila, exemplo_paciente(1)) == FILA_SEM_MEMORIA);
    assert(clinica.fila.inicio == NULL && clinica.fila.fim == NULL);
    liberar_clinica(&clinica);

    preparar_clinica(&clinica);
    inicio_anterior = clinica.fila.inicio;
    fim_anterior = clinica.fila.fim;
    falhar_proxima_alocacao();
    assert(colocar_paciente_na_fila(&clinica, 1) == CLINICA_SEM_MEMORIA);
    assert(clinica.fila.inicio == inicio_anterior && clinica.fila.fim == fim_anterior);
    falhar_proxima_alocacao();
    assert(atender_proximo_paciente(&clinica, &saida) == CLINICA_SEM_MEMORIA);
    conferir_paciente(saida.paciente, 99);
    assert(saida.medico.id_medico == 99);
    conferir_frente(&clinica, 1);
    conferir_atual(&clinica, 10);
    assert(clinica.historico.topo == NULL);
    assert(clinica.fila.inicio == inicio_anterior && clinica.fila.fim == fim_anterior);

    assert(atender_proximo_paciente(&clinica, &saida) == CLINICA_OK);
    inicio_anterior = clinica.fila.inicio;
    topo_anterior = clinica.historico.topo;
    saida = (Atendimento){{99, "Maria", 28, "Dor de joelho"}, {99, "Sentinela"}};
    falhar_proxima_alocacao();
    assert(desfazer_ultimo_atendimento(&clinica, &saida) == CLINICA_SEM_MEMORIA);
    conferir_paciente(saida.paciente, 99);
    assert(saida.medico.id_medico == 99 && strcmp(saida.medico.nome, "Sentinela") == 0);
    assert(clinica.fila.inicio == inicio_anterior && clinica.fila.fim == fim_anterior);
    assert(clinica.historico.topo == topo_anterior);
    conferir_frente(&clinica, 2);
    conferir_atual(&clinica, 20);
    assert(consultar_topo(&clinica.historico, &topo) == PILHA_OK);
    conferir_paciente(topo.paciente, 1);
    assert(desfazer_ultimo_atendimento(&clinica, &saida) == CLINICA_OK);
    conferir_paciente(saida.paciente, 1);
    conferir_frente(&clinica, 1);
    assert(clinica.historico.topo == NULL);
    liberar_clinica(&clinica);
    assert(alocacoes_pendentes() == 0);
    puts("OK: falhas nos quatro TADs preservam fila, escala e historico; zero alocacoes pendentes.");
}
#endif

int main(void)
{
    testar_cadastro_e_fila();
    testar_fluxo_completo();
#ifdef TESTAR_ALOCACAO
    testar_falhas_integracao();
    assert(alocacoes_pendentes() == 0);
#endif
    puts("Todos os testes de integracao passaram.");
    return 0;
}
