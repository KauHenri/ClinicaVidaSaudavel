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
        if ($Linhas.Count -gt 0) {
            $processo.StandardInput.Write(($Linhas -join "`n") + "`n")
        }
        $processo.StandardInput.Close()
        if (-not $processo.WaitForExit(10000)) {
            $processo.Kill()
            throw 'O menu nao terminou dentro de 10 segundos.'
        }
        $saida = $leituraSaida.GetAwaiter().GetResult()
        $erro = $leituraErro.GetAwaiter().GetResult()
        Exigir ($processo.ExitCode -eq 0) "Menu saiu com codigo $($processo.ExitCode): $erro"
        Exigir ([string]::IsNullOrEmpty($erro)) "Menu escreveu erros: $erro"
        Exigir ($saida.Contains('Memoria liberada. Fim do programa!')) 'Saida sem encerramento.'
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
    '1', '7', '8', '4', '4', '9', '5', '7', '4', '9',
    '6', '8', '5', '5', '5', '7', '9', '10', '0'
)
Conferir-Sequencia $saida @(
    'Dados de teste carregados com sucesso!',
    '1. ID: 101 | Nome: Maria Silva',
    '2. ID: 102 | Nome: Joao Souza',
    '[ATUAL] ID: 1',
    'Paciente: Maria Silva | Medico: Dr. Carlos (Clinico)',
    'Paciente: Joao Souza | Medico: Dra. Beatriz (Pediatra)',
    '[TOPO] Paciente: 102',
    'Medico: 2 | Nome: Dra. Beatriz (Pediatra)',
    'Paciente: 101',
    'Medico: 1 | Nome: Dr. Carlos (Clinico)',
    'Paciente Joao Souza voltou para o inicio da fila.',
    '1. ID: 102 | Nome: Joao Souza',
    'Paciente: Joao Souza | Medico: Dr. Carlos (Clinico)',
    '[TOPO] Paciente: 102',
    'Medico: 1 | Nome: Dr. Carlos (Clinico)',
    'Plantao avancado! Medico atual: Dr. Carlos (Clinico)',
    '[ATUAL] ID: 1',
    'Nenhum atendimento para desfazer.',
    '1. ID: 101 | Nome: Maria Silva',
    '2. ID: 102 | Nome: Joao Souza',
    'Nenhum atendimento registrado.',
    'ID: 101 | Nome: Maria Silva | Idade: 28'
)
Write-Output 'OK: exemplos, FIFO, escala, historico e desfazimentos consecutivos.'

$saida = Executar-Cenario @(
    '0', '1', '201', 'Lucas', '20', 'Dor', '1',
    '1', '202', 'Ana', '22', 'Febre', '0',
    '2', '999', '2', '202',
    '1', '201', 'Duplicado', '30', 'Dor',
    '3', '10', 'Dra. Joana', '3', '10', 'Outro nome',
    '7', '4', '5', '7', '8', '10', '0'
)
Conferir-Sequencia $saida @(
    'Paciente cadastrado com sucesso!',
    'Paciente adicionado na fila de espera!',
    'Paciente cadastrado com sucesso!',
    'Paciente nao encontrado no cadastro!',
    'Paciente Ana (ID 202) entrou na fila de espera.',
    'Ja existe um paciente com o ID 201!',
    'Medico Dra. Joana cadastrado no plantao!',
    'Medico com esse ID ja cadastrado.',
    '1. ID: 201 | Nome: Lucas',
    '2. ID: 202 | Nome: Ana',
    'Paciente: Lucas | Medico: Dra. Joana',
    'Paciente Lucas voltou para o inicio da fila.',
    '1. ID: 201 | Nome: Lucas',
    '2. ID: 202 | Nome: Ana'
)
Exigir ([regex]::Matches($saida, 'Paciente cadastrado com sucesso!').Count -eq 2) 'Paciente duplicado foi cadastrado.'
Write-Output 'OK: cadastro manual, encaminhamento opcional, busca e IDs duplicados.'

$saida = Executar-Cenario @(
    '0', '4', '5', '6', '7', '8', '9', '10',
    '3', '10', 'Ana', '4', '0'
)
Exigir ($saida.Contains('Nao ha medicos de plantao!')) 'Ausencia de medicos nao foi tratada.'
Exigir ($saida.Contains('Nenhum atendimento para desfazer.')) 'Historico vazio nao foi tratado.'
Exigir ($saida.Contains('Nenhum paciente na fila de espera!')) 'Fila vazia nao foi tratada.'
Exigir (-not $saida.Contains('Atendimento realizado com sucesso!')) 'Atendimento sem paciente foi realizado.'
Write-Output 'OK: bloqueios com fila, escala e historico vazios.'

$saida = Executar-Cenario @(
    'abc', '2', '999999999999999999999999999999', '1abc', '0',
    '11', '-1', '1', '0', '201', ('X' * 50), '   ', '  Lucas  ', '-1', '20',
    ('Y' * 100), 'Dor', '2', '0',
    '3', '-2', '10', ('M' * 50), 'Joana', '4', '10', '8', '0'
)
Exigir ([regex]::Matches($saida, 'Entrada invalida!').Count -eq 10) 'Validacao numerica incorreta.'
Exigir ([regex]::Matches($saida, 'Texto muito longo.').Count -eq 3) 'Limites de texto incorretos.'
Exigir ($saida.Contains('O campo nao pode ficar vazio.')) 'Campo vazio foi aceito.'
Exigir ($saida.Contains('ID: 201 | Nome: Lucas | Idade: 20 | Sintoma: Dor')) 'Cadastro valido nao foi concluido ou nome nao foi aparado.'
Exigir ($saida.Contains('[ATUAL] ID: 10 | Nome: Joana')) 'Medico valido nao foi cadastrado.'
Write-Output 'OK: entradas invalidas, limites numericos, campos vazios e textos longos.'

$saida = Executar-Cenario @(
    '0', '1', '1', ('N' * 49), '28', ('S' * 99), '1',
    '3', '10', ('M' * 49), '4', '1', '2'
)
Exigir ([regex]::Matches($saida, 'Paciente cadastrado com sucesso!').Count -eq 1) 'EOF gerou cadastro parcial.'
Exigir ($saida.Contains('Atendimento realizado com sucesso!')) 'Texto no limite nao foi aceito.'
Exigir (-not $saida.Contains('Texto muito longo.')) 'Texto no limite foi rejeitado.'
Write-Output 'OK: tamanhos maximos e EOF com historico ocupado.'

$cenariosEof = @(
    (Executar-Cenario @()),
    (Executar-Cenario @('0')),
    (Executar-Cenario @('0', '1')),
    (Executar-Cenario @('0', '1', '201')),
    (Executar-Cenario @('0', '1', '201', 'Lucas')),
    (Executar-Cenario @('0', '1', '201', 'Lucas', '20')),
    (Executar-Cenario @('0', '2')),
    (Executar-Cenario @('0', '3')),
    (Executar-Cenario @('0', '3', '10'))
)
foreach ($saida in $cenariosEof) {
    Exigir (-not $saida.Contains('Paciente cadastrado com sucesso!')) 'EOF criou paciente incompleto.'
    Exigir (-not $saida.Contains('cadastrado no plantao!')) 'EOF criou medico incompleto.'
}
Write-Output 'OK: EOF no inicio e durante paciente, medico e encaminhamento, sem repeticao infinita.'
