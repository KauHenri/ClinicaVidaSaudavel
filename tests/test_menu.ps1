param([Parameter(Mandatory = $true)][string]$Executavel)

$ErrorActionPreference = 'Stop'
$caminhoExecutavel = (Resolve-Path -LiteralPath $Executavel).Path

function Exigir {
    param([bool]$Condicao, [string]$Mensagem)
    if (-not $Condicao) { throw $Mensagem }
}

function Executar-Cenario {
    param([string[]]$Linhas)
    $configuracao = [System.Diagnostics.ProcessStartInfo]::new()
    $configuracao.FileName = $caminhoExecutavel
    $configuracao.UseShellExecute = $false
    $configuracao.CreateNoWindow = $true
    $configuracao.RedirectStandardInput = $true
    $configuracao.RedirectStandardOutput = $true
    $configuracao.RedirectStandardError = $true
    $configuracao.StandardOutputEncoding = [System.Text.UTF8Encoding]::new($false)
    $configuracao.StandardErrorEncoding = [System.Text.UTF8Encoding]::new($false)
    $processo = [System.Diagnostics.Process]::new()
    $processo.StartInfo = $configuracao
    try {
        Exigir ($processo.Start()) 'Nao foi possivel iniciar o menu.'
        # Ler em paralelo evita bloquear quando o menu ocupa o buffer de saida.
        $leituraSaida = $processo.StandardOutput.ReadToEndAsync()
        $leituraErro = $processo.StandardError.ReadToEndAsync()
        $processo.StandardInput.Write(($Linhas -join "`n") + "`n")
        $processo.StandardInput.Close()
        if (-not $processo.WaitForExit(10000)) {
            $processo.Kill()
            throw 'O menu nao terminou dentro de 10 segundos.'
        }
        $saida = $leituraSaida.GetAwaiter().GetResult()
        $erro = $leituraErro.GetAwaiter().GetResult()
        Exigir ($processo.ExitCode -eq 0) "Menu saiu com codigo $($processo.ExitCode): $erro"
        Exigir ([string]::IsNullOrEmpty($erro)) "Menu escreveu erros: $erro"
        Exigir ($saida.Contains('Programa encerrado. Memoria liberada.')) 'Saida sem encerramento.'
        return $saida.Replace("`r`n", "`n")
    } finally {
        $processo.Dispose()
    }
}

function Conferir-Sequencia {
    param([string]$Saida, [string[]]$Trechos)
    $indice = 0
    foreach ($trecho in $Trechos) {
        $posicao = $Saida.IndexOf($trecho, $indice, [System.StringComparison]::Ordinal)
        Exigir ($posicao -ge 0) "Saida esperada ausente ou fora de ordem: $trecho"
        $indice = $posicao + $trecho.Length
    }
}

$saida = Executar-Cenario @(
    '1', '1', 'Maria', '28', 'Dor de joelho',
    '1', '2', 'Carlos', '35', 'Febre',
    '2', '1', '4',
    '7', '10', 'Ana', '7', '20', 'Bruno',
    '11', '9', '8', '9', '5', '1', '5', '2', '6',
    '12', '8', '15', '12', '13', '6', '12', '14',
    '13', '13', '13', '3', '1', '2', '1', '10', '11', '10', '12', '0'
)
Conferir-Sequencia $saida @(
    'ID: 1 | Nome: Maria | Idade: 28',
    '[ATUAL] ID: 10 | Nome: Ana',
    'Medico atual: 20 | Nome: Bruno',
    'Medico atual: 10 | Nome: Ana',
    "Atendimento realizado.`nPaciente: 1 | Nome: Maria | Medico: 10 | Nome: Ana",
    'Medico atual: 20 | Nome: Bruno',
    'Ultimo atendimento (sem remover):',
    "Atendimento realizado.`nPaciente: 2 | Nome: Carlos | Medico: 20 | Nome: Bruno",
    'Atendimento desfeito. Paciente devolvido ao inicio da fila.',
    'da fila -> ID: 2 | Nome: Carlos',
    "Atendimento realizado.`nPaciente: 2 | Nome: Carlos | Medico: 10 | Nome: Ana",
    '[TOPO] Paciente: 2 | Nome: Carlos',
    'Paciente: 1 | Nome: Maria',
    'Nao ha atendimentos no historico.',
    'Paciente nao encontrado.',
    'Cadastre um medico antes de atender.'
)
Write-Output 'OK: todas as opcoes do menu, FIFO, escala, historico e desfazimento.'

$saida = Executar-Cenario @(
    'abc', '16', '-1', '999999999999999999999999999999',
    '1', '0', '1', ('X' * 50), '   ', 'Maria', '-1', '28',
    ('Y' * 100), 'Dor', '5', '999', '12',
    '7', '10', ('M' * 50), 'Ana', '12', '0'
)
Exigir ([regex]::Matches($saida, 'Entrada invalida').Count -eq 6) 'Validacao numerica incorreta.'
Exigir ([regex]::Matches($saida, 'Texto muito longo').Count -eq 3) 'Limites de texto incorretos.'
Exigir ($saida.Contains('O campo nao pode ficar vazio.')) 'Campo vazio foi aceito.'
Exigir ($saida.Contains('Cadastro atualizado com sucesso.')) 'Cadastro valido nao foi concluido.'
Exigir ($saida.Contains('Paciente nao cadastrado.')) 'ID inexistente foi aceito na fila.'
Exigir ([regex]::Matches($saida, 'Nao ha pacientes na fila.').Count -eq 2) 'Fila vazia nao foi tratada.'
Write-Output 'OK: entradas invalidas, overflow numerico, campos vazios e textos longos.'

$saida = Executar-Cenario @(
    '1', '1', ('N' * 49), '28', ('S' * 99),
    '7', '10', ('M' * 49), '5', '1', '12', '1', '2'
)
Exigir ([regex]::Matches($saida, 'Cadastro atualizado com sucesso.').Count -eq 1) 'EOF gerou cadastro parcial.'
Exigir ($saida.Contains('Atendimento realizado.')) 'Texto no limite nao foi aceito.'
Exigir (-not $saida.Contains('Texto muito longo.')) 'Texto no limite foi rejeitado.'
Write-Output 'OK: tamanhos maximos, EOF no cadastro e encerramento com estruturas ocupadas.'
