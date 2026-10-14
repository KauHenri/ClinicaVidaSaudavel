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

    # Apenas os objetos dos TADs usam o alocador de teste. O substituto chama
    # malloc/free reais, sem macros, evitando recursao e mudancas na producao.
    $substituicoes = @('-include', 'tests/substituir_alocacao.h')
    foreach ($modulo in @('medico', 'pilha')) {
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
    Write-Output "Validacao $sufixo concluida."
} finally {
    $env:PATH = $pathAnterior
    Set-Location -LiteralPath $diretorioAnterior
}
