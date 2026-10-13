#include "alocacao_teste.h"

#include <assert.h>
#include <stdlib.h>

static int falhar = 0;
static size_t pendentes = 0;

void *malloc_teste(size_t tamanho)
{
    void *endereco;
    if (falhar) {
        falhar = 0;
        return NULL;
    }
    endereco = malloc(tamanho);
    if (endereco != NULL) {
        ++pendentes;
    }
    return endereco;
}

void free_teste(void *endereco)
{
    if (endereco != NULL) {
        assert(pendentes > 0);
        --pendentes;
    }
    free(endereco);
}

void falhar_proxima_alocacao(void)
{
    falhar = 1;
}

size_t alocacoes_pendentes(void)
{
    return pendentes;
}
