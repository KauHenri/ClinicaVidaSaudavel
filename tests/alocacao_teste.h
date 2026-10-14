#ifndef ALOCACAO_TESTE_H
#define ALOCACAO_TESTE_H

#include <stddef.h>

/* Substitutos usados somente nos objetos de teste, nunca no programa final. */
void *malloc_teste(size_t tamanho);
void free_teste(void *endereco);
void falhar_proxima_alocacao(void);
size_t alocacoes_pendentes(void);

#endif
