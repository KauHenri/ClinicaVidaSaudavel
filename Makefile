# O alvo padrao compila o que mudou e abre o menu da clinica.
.DEFAULT_GOAL := run

CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror
CPPFLAGS ?= -I.
BUILD_DIR ?= build/make

ifeq ($(OS),Windows_NT)
SHELL := cmd.exe
.SHELLFLAGS := /C
EXE := .exe
EXECUTAR = $(subst /,\,$(1))
CRIAR_DIRETORIO = if not exist "$(subst /,\,$@)" mkdir "$(subst /,\,$@)"
POWERSHELL := $(SystemRoot)/System32/WindowsPowerShell/v1.0/powershell.exe
ifeq ($(origin CC),default)
CC := $(strip $(shell where gcc >NUL 2>&1 && echo gcc || echo clang))
endif
else
EXE :=
EXECUTAR = ./$(1)
CRIAR_DIRETORIO = mkdir -p "$@"
ifeq ($(origin CC),default)
CC := $(strip $(shell command -v gcc >/dev/null 2>&1 && echo gcc || echo clang))
endif
endif

MODULOS := clinica.c paciente.c fila.c medico.c pilha.c
FONTES := main.c $(MODULOS)
CABECALHOS := clinica.h paciente.h fila.h medico.h pilha.h
OBJETOS := $(addprefix $(BUILD_DIR)/,$(FONTES:.c=.o))
OBJETOS_FALHAS := $(addprefix $(BUILD_DIR)/falhas/,paciente.o fila.o medico.o pilha.o)
PROGRAMA := $(BUILD_DIR)/clinica$(EXE)
TESTES := $(addprefix $(BUILD_DIR)/test_,estruturas$(EXE) integracao$(EXE))
TESTES_FALHAS := $(addprefix $(BUILD_DIR)/test_falhas_,estruturas$(EXE) integracao$(EXE))

.PHONY: run build test test-asan

run: $(PROGRAMA)
	@"$(call EXECUTAR,$(PROGRAMA))"

build: $(PROGRAMA)

$(PROGRAMA): $(OBJETOS)
	"$(CC)" $(LDFLAGS) $(OBJETOS) -o "$@" $(LDLIBS)

# Cada .c vira um .o. Os arquivos .d registram as dependencias dos .h.
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	"$(CC)" $(CPPFLAGS) $(CFLAGS) -MMD -MP -c "$<" -o "$@"

$(BUILD_DIR):
	@$(CRIAR_DIRETORIO)

$(BUILD_DIR)/falhas: | $(BUILD_DIR)
	@$(CRIAR_DIRETORIO)

$(BUILD_DIR)/test_%$(EXE): tests/test_%.c $(MODULOS) $(CABECALHOS) | $(BUILD_DIR)
	"$(CC)" $(CPPFLAGS) $(CFLAGS) "$<" $(MODULOS) $(LDFLAGS) -o "$@" $(LDLIBS)

# O alocador de teste substitui malloc/free somente nos quatro TADs.
$(BUILD_DIR)/falhas/%.o: %.c tests/substituir_alocacao.h tests/alocacao_teste.h $(CABECALHOS) | $(BUILD_DIR)/falhas
	"$(CC)" $(CPPFLAGS) $(CFLAGS) -include tests/substituir_alocacao.h -c "$<" -o "$@"

$(BUILD_DIR)/test_falhas_%$(EXE): tests/test_%.c tests/alocacao_teste.c tests/alocacao_teste.h clinica.c $(CABECALHOS) $(OBJETOS_FALHAS) | $(BUILD_DIR)
	"$(CC)" $(CPPFLAGS) $(CFLAGS) -DTESTAR_ALOCACAO "$<" tests/alocacao_teste.c clinica.c $(OBJETOS_FALHAS) $(LDFLAGS) -o "$@" $(LDLIBS)

test: $(TESTES) $(TESTES_FALHAS) $(PROGRAMA)
	@"$(call EXECUTAR,$(BUILD_DIR)/test_estruturas$(EXE))"
	@"$(call EXECUTAR,$(BUILD_DIR)/test_integracao$(EXE))"
	@"$(call EXECUTAR,$(BUILD_DIR)/test_falhas_estruturas$(EXE))"
	@"$(call EXECUTAR,$(BUILD_DIR)/test_falhas_integracao$(EXE))"
ifeq ($(OS),Windows_NT)
	@"$(POWERSHELL)" -NoProfile -ExecutionPolicy Bypass -File tests/test_menu.ps1 -Executavel "$(PROGRAMA)"
endif

# Uma pasta separada evita misturar objetos normais e instrumentados.
test-asan:
	+"$(MAKE)" test BUILD_DIR=build/make-asan CFLAGS="$(CFLAGS) -fsanitize=address -fno-omit-frame-pointer -g" LDFLAGS="$(LDFLAGS) -fsanitize=address"

-include $(OBJETOS:.o=.d)
