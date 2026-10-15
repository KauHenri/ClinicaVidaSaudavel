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
        Exigir ($saida.Contains('Sistema da Clinica Vida Saudavel encerrado. Ate logo!')) 'Saida sem encerramento.'
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
    '6', '8', '5', '5', '5', '10', '0'
)
Conferir-Sequencia $saida @(
    'DADOS DE EXEMPLO CARREGADOS COM SUCESSO!',
    '[ATUAL] ID: 1',
    'Paciente: Maria Silva (ID 101, 28 anos)',
    'Dr. Carlos Eduardo (Clinico Geral) (ID 1)',
    'Paciente: Joao Souza (ID 102, 45 anos)',
    'Dra. Beatriz Santos (Pediatra) (ID 2)',
    '[TOPO] Paciente: 102',
    'Paciente: Joao Souza (ID 102) voltou para a FRENTE',
    'da fila -> ID: 102 | Nome: Joao Souza',
    'Paciente: Joao Souza (ID 102, 45 anos)',
    'Dr. Andre Oliveira (Ortopedista) (ID 3)',
    '[TOPO] Paciente: 102',
    'Medico da vez agora: Dra. Beatriz Santos (Pediatra) (ID 2)',
    '[ATUAL] ID: 2',
    'Nenhum atendimento registrado no historico para desfazer!',
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
    "Paciente 'Lucas' cadastrado",
    'Paciente colocado na Sala de Espera (Fila FIFO)!',
    "Paciente 'Ana' cadastrado",
    'Paciente com ID 999 nao encontrado',
    "Paciente 'Ana' (ID 202) entrou na Sala de Espera!",
    'Ja existe um paciente cadastrado com o ID 201!',
    "Medico 'Dra. Joana' inserido",
    'Ja existe um medico cadastrado com o ID 10!',
    'da fila -> ID: 201 | Nome: Lucas',
    'da fila -> ID: 202 | Nome: Ana',
    'Paciente: Lucas (ID 201, 20 anos)',
    'Paciente: Lucas (ID 201) voltou para a FRENTE',
    'da fila -> ID: 201 | Nome: Lucas',
    'da fila -> ID: 202 | Nome: Ana'
)
Write-Output 'OK: cadastro manual, encaminhamento opcional, busca e IDs duplicados.'

$saida = Executar-Cenario @(
    '0', '4', '5', '6', '7', '8', '9', '10',
    '3', '10', 'Ana', '4', '0'
)
Exigir ($saida.Contains('Nenhum medico de plantao')) 'Ausencia de medicos nao foi tratada.'
Exigir ($saida.Contains('Nenhum atendimento registrado no historico para desfazer!')) 'Historico vazio nao foi tratado.'
Exigir ($saida.Contains('A Sala de Espera esta vazia!')) 'Fila vazia nao foi tratada.'
Exigir (-not $saida.Contains('ATENDIMENTO CONCLUIDO COM SUCESSO!')) 'Atendimento sem paciente foi realizado.'
Write-Output 'OK: bloqueios com fila, escala e historico vazios.'

$saida = Executar-Cenario @(
    'abc', '2', '999999999999999999999999999999', '1abc', '0',
    '11', '-1', '1', '0', '201', ('X' * 50), '   ', '  Lucas  ', '-1', '20',
    ('Y' * 100), 'Dor', '2', '0',
    '3', '-2', '10', ('M' * 50), 'Joana', '4', '0'
)
Exigir ([regex]::Matches($saida, 'Entrada invalida!').Count -eq 10) 'Validacao numerica incorreta.'
Exigir ([regex]::Matches($saida, 'Texto muito longo.').Count -eq 3) 'Limites de texto incorretos.'
Exigir ($saida.Contains('O campo nao pode ficar vazio.')) 'Campo vazio foi aceito.'
Exigir ($saida.Contains("Paciente 'Lucas' cadastrado")) 'Cadastro valido nao foi concluido ou nome nao foi aparado.'
Exigir ($saida.Contains("Medico 'Joana' inserido")) 'Medico valido nao foi cadastrado.'
Write-Output 'OK: entradas invalidas, limites numericos, campos vazios e textos longos.'

$saida = Executar-Cenario @(
    '0', '1', '1', ('N' * 49), '28', ('S' * 99), '1',
    '3', '10', ('M' * 49), '4', '1', '2'
)
Exigir ([regex]::Matches($saida, '\[SUCESSO\] Paciente').Count -eq 1) 'EOF gerou cadastro parcial.'
Exigir ($saida.Contains('ATENDIMENTO CONCLUIDO COM SUCESSO!')) 'Texto no limite nao foi aceito.'
Exigir (-not $saida.Contains('Texto muito longo.')) 'Texto no limite foi rejeitado.'
Write-Output 'OK: tamanhos maximos e EOF com historico ocupado.'

$cenariosEof = @(
    (Executar-Cenario @()),
    (Executar-Cenario @('0')),
    (Executar-Cenario @('0', '1')),
    (Executar-Cenario @('0', '1', '201', 'Lucas', '20')),
    (Executar-Cenario @('0', '2')),
    (Executar-Cenario @('0', '3', '10'))
)
foreach ($saida in $cenariosEof) {
    Exigir (-not $saida.Contains('[SUCESSO] Paciente')) 'EOF criou paciente incompleto.'
    Exigir (-not $saida.Contains('[SUCESSO] Medico')) 'EOF criou medico incompleto.'
}
Write-Output 'OK: EOF no inicio e durante paciente, medico e encaminhamento, sem repeticao infinita.'
