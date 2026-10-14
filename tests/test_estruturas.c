#include "medico.h"
#include "pilha.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef TESTAR_ALOCACAO
#include "alocacao_teste.h"
#endif

static void conferir_medico(const ListaMedicos *lista, int id)
{
    Medico atual;
    assert(consultar_medico_atual(lista, &atual) == MEDICO_OK);
    assert(atual.id_medico == id);
}

static void conferir_texto(FILE *arquivo, const char *esperado)
{
    char texto[2048];
    size_t tamanho;
    assert(fflush(arquivo) == 0);
    rewind(arquivo);
    tamanho = fread(texto, 1, sizeof texto - 1, arquivo);
    assert(!ferror(arquivo));
    assert(feof(arquivo));
    texto[tamanho] = '\0';
    assert(strcmp(texto, esperado) == 0);
    assert(fclose(arquivo) == 0);
}

static void testar_medicos(void)
{
    ListaMedicos lista;
    Medico saida = {99, "Sentinela"};
    Medico invalido = {4, ""};
    FILE *arquivo;
    int volta;

    inicializar_medicos(&lista);
    assert(lista.atual == NULL);
    assert(avancar_medico(&lista) == MEDICO_LISTA_VAZIA);
    assert(remover_medico_atual(&lista) == MEDICO_LISTA_VAZIA);
    assert(consultar_medico_atual(&lista, &saida) == MEDICO_LISTA_VAZIA);
    assert(saida.id_medico == 99 && strcmp(saida.nome, "Sentinela") == 0);
    arquivo = tmpfile();
    assert(arquivo != NULL);
    listar_medicos(&lista, arquivo);
    conferir_texto(arquivo, "Nenhum medico de plantao.\n");
    liberar_medicos(&lista);

    assert(inserir_medico(&lista, (Medico){1, "Ana"}) == MEDICO_OK);
    assert(avancar_medico(&lista) == MEDICO_OK);
    conferir_medico(&lista, 1);
    assert(inserir_medico(&lista, (Medico){2, "Bruno"}) == MEDICO_OK);
    assert(inserir_medico(&lista, (Medico){3, "Clara"}) == MEDICO_OK);
    assert(inserir_medico(&lista, (Medico){2, "Outro nome"}) == MEDICO_ID_DUPLICADO);

    /* Repetir varias voltas verifica os enlaces, sem inspecionar nos privados. */
    for (volta = 0; volta < 8; ++volta) {
        int id;
        for (id = 1; id <= 3; ++id) {
            conferir_medico(&lista, id);
            assert(avancar_medico(&lista) == MEDICO_OK);
        }
    }
    assert(avancar_medico(&lista) == MEDICO_OK);
    conferir_medico(&lista, 2);
    arquivo = tmpfile();
    assert(arquivo != NULL);
    listar_medicos(&lista, arquivo);
    conferir_texto(arquivo, "[ATUAL] ID: 2 | Nome: Bruno\n"
                            "ID: 3 | Nome: Clara\n"
                            "ID: 1 | Nome: Ana\n");
    conferir_medico(&lista, 2);

    /* Inserir com o atual ja rotacionado nao deve trocar o plantonista. */
    assert(inserir_medico(&lista, (Medico){4, "Daniel"}) == MEDICO_OK);
    conferir_medico(&lista, 2);
    assert(remover_medico_atual(&lista) == MEDICO_OK);
    conferir_medico(&lista, 3);
    assert(avancar_medico(&lista) == MEDICO_OK);
    conferir_medico(&lista, 1);
    assert(avancar_medico(&lista) == MEDICO_OK);
    conferir_medico(&lista, 4);
    assert(avancar_medico(&lista) == MEDICO_OK);
    conferir_medico(&lista, 3);
    assert(remover_medico_atual(&lista) == MEDICO_OK);
    conferir_medico(&lista, 1);
    assert(remover_medico_atual(&lista) == MEDICO_OK);
    conferir_medico(&lista, 4);
    assert(avancar_medico(&lista) == MEDICO_OK);
    conferir_medico(&lista, 4);
    assert(remover_medico_atual(&lista) == MEDICO_OK);
    assert(lista.atual == NULL);
    assert(remover_medico_atual(&lista) == MEDICO_LISTA_VAZIA);

    /* A liberacao precisa funcionar tanto com um no quanto com varios. */
    assert(inserir_medico(&lista, (Medico){1, "Ana"}) == MEDICO_OK);
    liberar_medicos(&lista);
    assert(lista.atual == NULL);
    assert(inserir_medico(&lista, (Medico){1, "Ana"}) == MEDICO_OK);
    assert(inserir_medico(&lista, (Medico){2, "Bruno"}) == MEDICO_OK);
    assert(inserir_medico(&lista, (Medico){3, "Clara"}) == MEDICO_OK);
    assert(avancar_medico(&lista) == MEDICO_OK);
    liberar_medicos(&lista);
    assert(lista.atual == NULL);
    liberar_medicos(&lista);

    memset(invalido.nome, 'X', sizeof invalido.nome);
    assert(inserir_medico(&lista, invalido) == MEDICO_ARGUMENTO_INVALIDO);
    assert(lista.atual == NULL);
    assert(inserir_medico(NULL, (Medico){1, "Ana"}) == MEDICO_ARGUMENTO_INVALIDO);
    assert(avancar_medico(NULL) == MEDICO_ARGUMENTO_INVALIDO);
    assert(remover_medico_atual(NULL) == MEDICO_ARGUMENTO_INVALIDO);
    assert(consultar_medico_atual(NULL, &saida) == MEDICO_ARGUMENTO_INVALIDO);
    assert(consultar_medico_atual(&lista, NULL) == MEDICO_ARGUMENTO_INVALIDO);
    inicializar_medicos(NULL);
    liberar_medicos(NULL);
    listar_medicos(NULL, stdout);
    listar_medicos(&lista, NULL);
    puts("OK: lista circular, rotacao, listagem, remocoes e liberacao.");
}

static Atendimento exemplo(int id)
{
    Atendimento dados = {{id, "Maria", 28, "Dor de joelho"}, {10, "Ana"}};
    return dados;
}

static void conferir_atendimento(const Atendimento *dados, int id)
{
    assert(dados->paciente.id_paciente == id);
    assert(strcmp(dados->paciente.nome, "Maria") == 0);
    assert(dados->paciente.idade == 28);
    assert(strcmp(dados->paciente.sintoma, "Dor de joelho") == 0);
    assert(dados->medico.id_medico == 10);
    assert(strcmp(dados->medico.nome, "Ana") == 0);
}

static void testar_pilha(void)
{
    PilhaAtendimentos pilha;
    Atendimento saida = exemplo(99);
    Atendimento origem = exemplo(1);
    Atendimento invalido;
    FILE *arquivo;
    int id;

    inicializar_pilha(&pilha);
    assert(pilha.topo == NULL);
    assert(consultar_topo(&pilha, &saida) == PILHA_VAZIA);
    conferir_atendimento(&saida, 99);
    assert(desempilhar_atendimento(&pilha, &saida) == PILHA_VAZIA);
    conferir_atendimento(&saida, 99);
    arquivo = tmpfile();
    assert(arquivo != NULL);
    listar_atendimentos(&pilha, arquivo);
    conferir_texto(arquivo, "Nenhum atendimento registrado.\n");
    liberar_pilha(&pilha);

    assert(empilhar_atendimento(&pilha, origem) == PILHA_OK);
    /* Mudar os dados de origem nao pode mudar a fotografia armazenada. */
    origem.paciente.id_paciente = 500;
    strcpy(origem.paciente.nome, "Alterado");
    strcpy(origem.medico.nome, "Alterado");
    assert(consultar_topo(&pilha, &saida) == PILHA_OK);
    conferir_atendimento(&saida, 1);
    for (id = 2; id <= 3; ++id) {
        assert(empilhar_atendimento(&pilha, exemplo(id)) == PILHA_OK);
    }
    assert(consultar_topo(&pilha, &saida) == PILHA_OK);
    conferir_atendimento(&saida, 3);
    saida.paciente.id_paciente = 800;
    assert(consultar_topo(&pilha, &saida) == PILHA_OK);
    conferir_atendimento(&saida, 3);
    arquivo = tmpfile();
    assert(arquivo != NULL);
    listar_atendimentos(&pilha, arquivo);
    conferir_texto(arquivo,
        "[TOPO] Paciente: 3 | Nome: Maria | Idade: 28 | Sintoma: Dor de joelho | Medico: 10 | Nome: Ana\n"
        "Paciente: 2 | Nome: Maria | Idade: 28 | Sintoma: Dor de joelho | Medico: 10 | Nome: Ana\n"
        "Paciente: 1 | Nome: Maria | Idade: 28 | Sintoma: Dor de joelho | Medico: 10 | Nome: Ana\n");
    assert(consultar_topo(&pilha, &saida) == PILHA_OK);
    conferir_atendimento(&saida, 3);
    assert(desempilhar_atendimento(&pilha, NULL) == PILHA_ARGUMENTO_INVALIDO);
    for (id = 3; id >= 1; --id) {
        assert(desempilhar_atendimento(&pilha, &saida) == PILHA_OK);
        /* Estes dados sobrevivem ao free e permitem reconstruir o paciente. */
        conferir_atendimento(&saida, id);
    }
    assert(pilha.topo == NULL);
    assert(desempilhar_atendimento(&pilha, &saida) == PILHA_VAZIA);
    conferir_atendimento(&saida, 1);

    assert(empilhar_atendimento(&pilha, exemplo(1)) == PILHA_OK);
    liberar_pilha(&pilha);
    assert(pilha.topo == NULL);
    for (id = 1; id <= 32; ++id) {
        assert(empilhar_atendimento(&pilha, exemplo(id)) == PILHA_OK);
    }
    liberar_pilha(&pilha);
    assert(pilha.topo == NULL);
    liberar_pilha(&pilha);

    invalido = exemplo(1);
    memset(invalido.paciente.nome, 'X', sizeof invalido.paciente.nome);
    assert(empilhar_atendimento(&pilha, invalido) == PILHA_ARGUMENTO_INVALIDO);
    invalido = exemplo(1);
    memset(invalido.paciente.sintoma, 'X', sizeof invalido.paciente.sintoma);
    assert(empilhar_atendimento(&pilha, invalido) == PILHA_ARGUMENTO_INVALIDO);
    invalido = exemplo(1);
    memset(invalido.medico.nome, 'X', sizeof invalido.medico.nome);
    assert(empilhar_atendimento(&pilha, invalido) == PILHA_ARGUMENTO_INVALIDO);
    assert(pilha.topo == NULL);
    assert(empilhar_atendimento(NULL, exemplo(1)) == PILHA_ARGUMENTO_INVALIDO);
    assert(desempilhar_atendimento(NULL, &saida) == PILHA_ARGUMENTO_INVALIDO);
    assert(consultar_topo(NULL, &saida) == PILHA_ARGUMENTO_INVALIDO);
    assert(consultar_topo(&pilha, NULL) == PILHA_ARGUMENTO_INVALIDO);
    inicializar_pilha(NULL);
    liberar_pilha(NULL);
    listar_atendimentos(NULL, stdout);
    listar_atendimentos(&pilha, NULL);
    puts("OK: pilha, PUSH, PEEK, POP em ordem LIFO e copias para desfazimento.");
}

static void testar_copias_entre_modulos(void)
{
    ListaMedicos lista;
    PilhaAtendimentos pilha;
    Atendimento registro = exemplo(1);
    Atendimento saida;

    inicializar_medicos(&lista);
    inicializar_pilha(&pilha);
    assert(inserir_medico(&lista, (Medico){10, "Ana"}) == MEDICO_OK);
    assert(consultar_medico_atual(&lista, &registro.medico) == MEDICO_OK);
    assert(empilhar_atendimento(&pilha, registro) == PILHA_OK);
    /* Mesmo depois do free do medico, o historico precisa continuar valido. */
    assert(remover_medico_atual(&lista) == MEDICO_OK);
    assert(consultar_topo(&pilha, &saida) == PILHA_OK);
    conferir_atendimento(&saida, 1);
    assert(desempilhar_atendimento(&pilha, &saida) == PILHA_OK);
    liberar_medicos(&lista);
    liberar_pilha(&pilha);
    conferir_atendimento(&saida, 1);
    puts("OK: historico preservado apos remover o medico e liberar as estruturas.");
}

#ifdef TESTAR_ALOCACAO
static void testar_falhas_alocacao(void)
{
    ListaMedicos lista;
    PilhaAtendimentos pilha;
    Atendimento saida;
    inicializar_medicos(&lista);
    inicializar_pilha(&pilha);

    falhar_proxima_alocacao();
    assert(inserir_medico(&lista, (Medico){1, "Ana"}) == MEDICO_SEM_MEMORIA);
    assert(lista.atual == NULL);
    assert(inserir_medico(&lista, (Medico){1, "Ana"}) == MEDICO_OK);
    falhar_proxima_alocacao();
    assert(inserir_medico(&lista, (Medico){2, "Bruno"}) == MEDICO_SEM_MEMORIA);
    conferir_medico(&lista, 1);
    assert(avancar_medico(&lista) == MEDICO_OK);
    conferir_medico(&lista, 1);
    assert(inserir_medico(&lista, (Medico){2, "Bruno"}) == MEDICO_OK);
    assert(avancar_medico(&lista) == MEDICO_OK);
    conferir_medico(&lista, 2);
    liberar_medicos(&lista);

    falhar_proxima_alocacao();
    assert(empilhar_atendimento(&pilha, exemplo(1)) == PILHA_SEM_MEMORIA);
    assert(pilha.topo == NULL);
    assert(empilhar_atendimento(&pilha, exemplo(1)) == PILHA_OK);
    falhar_proxima_alocacao();
    assert(empilhar_atendimento(&pilha, exemplo(2)) == PILHA_SEM_MEMORIA);
    assert(consultar_topo(&pilha, &saida) == PILHA_OK);
    conferir_atendimento(&saida, 1);
    assert(empilhar_atendimento(&pilha, exemplo(2)) == PILHA_OK);
    assert(desempilhar_atendimento(&pilha, &saida) == PILHA_OK);
    conferir_atendimento(&saida, 2);
    liberar_pilha(&pilha);
    assert(alocacoes_pendentes() == 0);
    puts("OK: falhas de malloc preservam os dados; nenhuma alocacao pendente nos TADs.");
}
#endif

int main(void)
{
    testar_medicos();
    testar_pilha();
    testar_copias_entre_modulos();
#ifdef TESTAR_ALOCACAO
    testar_falhas_alocacao();
#endif
    puts("Todos os testes passaram.");
    return 0;
}
