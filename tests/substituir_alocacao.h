#ifndef SUBSTITUIR_ALOCACAO_H
#define SUBSTITUIR_ALOCACAO_H

/* Incluimos a biblioteca antes das macros para nao modificar suas
 * declaracoes, inclusive os atributos de importacao usados no Windows. */
#include <stdlib.h>
#include "alocacao_teste.h"

#define malloc malloc_teste
#define free free_teste

#endif
