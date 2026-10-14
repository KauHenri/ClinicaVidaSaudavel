param(
    [string]$Compilador = 'gcc',
    [switch]$Sanitizador
)

$ErrorActionPreference = 'Stop'
$raizProjeto = Split-Path -Parent $PSScriptRoot
$diretorioAnterior = Get-Location
$pathAnterior = $env:PATH

function Executar-Comando {
    param([string]$Programa, [string[]]$Argumentos)
    & $Programa @Argumentos
    if ($LASTEXITCODE -ne 0) {
        throw "Falha em $Programa (codigo $LASTEXITCODE)."
    }
}

try {
    Set-Location -LiteralPath $raizProjeto
    $comandoCompilador = (Get-Command $Compilador -ErrorAction Stop).Source
    # O ASan do LLVM-MinGW encontra suas DLLs no diretorio do compilador.
    $env:PATH = (Split-Path -Parent $comandoCompilador) + ';' + $pathAnterior
    $sufixo = if ($Sanitizador) { 'asan' } else { 'normal' }
    $pastaBuild = "build/$sufixo"
    New-Item -ItemType Directory -Path $pastaBuild -Force | Out-Null

    $opcoes = @('-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-I.')
    if ($Sanitizador) {
        $opcoes += @('-fsanitize=address', '-fno-omit-frame-pointer', '-g')
    }

    Executar-Comando $comandoCompilador ($opcoes + @(
        'medico.c', 'pilha.c', 'tests/test_estruturas.c',
        '-o', "$pastaBuild/test_estruturas.exe"
    ))
    Executar-Comando "./$pastaBuild/test_estruturas.exe" @()

    $modulosClinica = @('clinica.c', 'paciente.c', 'fila.c', 'medico.c', 'pilha.c')
    Executar-Comando $comandoCompilador ($opcoes + $modulosClinica + @(
        'tests/test_integracao.c', '-o', "$pastaBuild/test_integracao.exe"
    ))
    Executar-Comando "./$pastaBuild/test_integracao.exe" @()
    Executar-Comando $comandoCompilador ($opcoes + $modulosClinica + @(
        'main.c', '-o', "$pastaBuild/clinica.exe"
    ))
    & "$PSScriptRoot/test_menu.ps1" -Executavel "./$pastaBuild/clinica.exe"

    # Apenas os objetos dos quatro TADs usam o alocador de teste. O substituto chama
    # malloc/free reais, sem macros, evitando recursao e mudancas na producao.
    $substituicoes = @('-include', 'tests/substituir_alocacao.h')
    foreach ($modulo in @('medico', 'pilha', 'paciente', 'fila')) {
        Executar-Comando $comandoCompilador ($opcoes + $substituicoes + @(
            '-c', "$modulo.c", '-o', "$pastaBuild/$modulo.o"
        ))
    }
    Executar-Comando $comandoCompilador ($opcoes + @(
        '-DTESTAR_ALOCACAO', 'tests/test_estruturas.c', 'tests/alocacao_teste.c',
        "$pastaBuild/medico.o", "$pastaBuild/pilha.o",
        '-o', "$pastaBuild/test_falhas.exe"
    ))
    Executar-Comando "./$pastaBuild/test_falhas.exe" @()
    Executar-Comando $comandoCompilador ($opcoes + @(
        '-DTESTAR_ALOCACAO', 'tests/test_integracao.c', 'tests/alocacao_teste.c', 'clinica.c',
        "$pastaBuild/medico.o", "$pastaBuild/pilha.o", "$pastaBuild/paciente.o", "$pastaBuild/fila.o",
        '-o', "$pastaBuild/test_falhas_integracao.exe"
    ))
    Executar-Comando "./$pastaBuild/test_falhas_integracao.exe" @()
    Write-Output "Validacao $sufixo concluida."
} finally {
    $env:PATH = $pathAnterior
    Set-Location -LiteralPath $diretorioAnterior
}
