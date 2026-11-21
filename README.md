# Controle da Lampisteria

Trabalho 2 da disciplina de **Introdução à Programação de Computadores** (CEFET-MG — Divinópolis, Engenharia de Computação), ministrada pelo Prof. Guido Pantuza.

O sistema gerencia o empréstimo, devolução, manutenção e controle de equipamentos da lampisteria (como lanternas, autorresgatadores, detectores de gás e rádios), organizando as solicitações dos mineiros em fila, registrando as operações recentes e o histórico das movimentações, além de salvar e carregar os dados em arquivos binários.

## Estrutura do projeto

Separei o código em módulos `.h` e `.c` para manter cada parte com sua responsabilidade:

- `main.c`: loop do menu principal (opções `0` a `12`), leitura de entradas (números com `scanf`, textos com `fgets`) e chamadas para os módulos.
- `equipamentos.h` / `equipamentos.c`: lista encadeada dinâmica de equipamentos mantida em ordem crescente de `id`, com verificação de ID repetido, busca, listagem e liberação de memória.
- `solicitacoes.h` / `solicitacoes.c`: fila dinâmica (FIFO) de solicitações com numeração sequencial de protocolo a partir de `1`, atendimento pela ordem de chegada, recolocação no início ou fim quando não há item disponível e cancelamento por protocolo sem alterar a ordem dos demais pedidos.
- `operacoes.h` / `operacoes.c`: pilha dinâmica para as operações concluídas com sucesso, exibindo sempre a mais recente no topo.
- `historico.h` / `historico.c`: lista simplesmente encadeada em ordem cronológica para registrar as movimentações concluídas (empréstimo, devolução, entrada/saída de manutenção e cancelamento).
- `arquivos.h` / `arquivos.c`: gravação e leitura em arquivos binários (`equipamentos.bin` e `historico.bin`) usando `fwrite` e `fread`.

## Decisões de projeto

- **Lista ordenada por ID**: escolhi inserir os equipamentos já ordenados por identificador na lista encadeada para facilitar a busca por ID e garantir que o atendimento pegue o primeiro equipamento disponível da categoria pedida na ordem crescente de ID.
- **Arquivos binários sem salvar ponteiros**: na persistência em arquivo, gravo apenas os campos de dados dos registros (usando structs próprias para o arquivo, sem o campo `prox`) e reconstruo os nós dinamicamente ao carregar, evitando endereços de memória inválidos entre execuções.
- **Carregamento seguro**: ao carregar, os dados são lidos para uma lista nova. Só depois de a leitura terminar sem erro a estrutura antiga é liberada e substituída. Assim, um arquivo corrompido ou incompleto não apaga o que já estava em memória.
- **Apenas operações válidas na pilha**: resolvi empilhar somente as ações concluídas com êxito para que tentativas inválidas (como ID repetido, equipamento inexistente, devolução de item não emprestado ou protocolo inexistente) não entrem na pilha de operações recentes.
- **Leitura completa antes de validar**: nas opções de cadastro, busca e cancelamento, o programa lê todos os dados digitados antes de validar. Isso mantém a entrada sincronizada mesmo quando a operação é recusada, o que permite rodar o programa com entrada redirecionada de arquivo.
- **Solicitação duplicada na fila**: a regra "não emprestar duas vezes a mesma categoria ao mesmo solicitante" é verificada ao registrar e também ao atender. Se a mesma matrícula entrou duas vezes na fila para a mesma categoria e já recebeu o primeiro equipamento, a segunda solicitação é descartada no atendimento, com uma mensagem de erro.
- **Fim da entrada (EOF)**: se a entrada padrão acabar, o programa libera a memória e encerra, em vez de ficar repetindo o menu.

## Como compilar e rodar

No terminal, dentro da pasta do projeto:

```bash
gcc -Wall -Wextra main.c equipamentos.c solicitacoes.c operacoes.c historico.c arquivos.c -o lampisteria
```

Para executar:

```bash
./lampisteria
```

No Windows, use `-o lampisteria.exe` na compilação e execute com `.\lampisteria.exe`.

## Instruções de uso

O programa mostra um menu em loop, que só termina com a opção `0`. Digite o número da opção e pressione Enter.

| Opção | Operação |
|-------|----------|
| 1 | Cadastrar equipamento (ID, descrição, categoria e código de patrimônio) |
| 2 | Listar equipamentos, com estado e responsável |
| 3 | Buscar equipamento por ID ou listar uma categoria |
| 4 | Registrar solicitação (nome, matrícula, categoria, local de trabalho e data) |
| 5 | Listar a fila de solicitações pendentes |
| 6 | Atender a próxima solicitação da fila |
| 7 | Registrar devolução de um equipamento emprestado |
| 8 | Colocar um equipamento em manutenção ou trazê-lo de volta |
| 9 | Cancelar uma solicitação pelo protocolo |
| 10 | Mostrar equipamentos, fila, operações recentes e histórico |
| 11 | Salvar dados em arquivos binários |
| 12 | Carregar dados dos arquivos binários |
| 0 | Sair (pergunta se deseja salvar e libera toda a memória) |

Observações de uso:

- A data deve ser digitada no formato `dd/mm/aaaa` (por exemplo, `05/10/2026`).
- A categoria deve ser digitada exatamente como foi cadastrada (por exemplo, `Lanterna`), respeitando maiúsculas e minúsculas.
- Quando não há equipamento disponível ao atender uma solicitação, o programa pergunta se ela deve ser mantida no início da fila (1), recolocada no final (2) ou cancelada (3).

## Testes realizados

Os arquivos `teste1.txt` e `teste2.txt` guardam as entradas dos testes. Cada linha do arquivo é uma entrada digitada.

Linux ou cmd do Windows:

```bash
./lampisteria < teste1.txt
./lampisteria < teste2.txt
```

PowerShell:

```powershell
Get-Content teste1.txt | .\lampisteria.exe
Get-Content teste2.txt | .\lampisteria.exe
```

Rode o `teste1.txt` primeiro, porque ele grava os arquivos `.bin` que o `teste2.txt` lê.

- **`teste1.txt`** reproduz a sessão dos Exemplos 1 a 5 do enunciado e termina salvando os dados. Resultados conferidos:
  - cadastro de 3 equipamentos e recusa do ID `101` repetido, com listagem em ordem de ID;
  - registro de 3 solicitações (protocolos 1, 2 e 3) e listagem da fila;
  - atendimento dos protocolos 1 e 2, e recolocação do protocolo 3 no final da fila por falta de Detector de gás;
  - erros tratados: devolução de equipamento não emprestado, manutenção de equipamento emprestado, solicitação de categoria já emprestada ao mesmo solicitante, equipamento inexistente (`150`) e cancelamento de protocolo inexistente ou com a fila vazia;
  - estado final com 3 equipamentos, fila vazia, 12 operações na pilha e 5 movimentações no histórico.
- **`teste2.txt`** simula uma nova execução: lista (sistema vazio), carrega os arquivos e lista de novo. Resultado esperado: `Dados carregados: 3 equipamentos, 5 movimentacoes.` com os mesmos estados de antes.

Também testei manualmente entrada inválida no menu (texto no lugar de número), carregamento sem os arquivos `.bin` existentes (mensagem de erro de abertura) e fim da entrada.

## Limitações conhecidas

- Apenas os equipamentos e o histórico são salvos em arquivo. A fila de solicitações e a pilha de operações recentes não são gravadas e começam vazias a cada execução.
- O contador de protocolos não é salvo, então volta a `1` em uma nova execução.
- A comparação de categorias diferencia maiúsculas de minúsculas (`Lanterna` e `lanterna` são tratadas como categorias diferentes).
- Salvar sobrescreve os arquivos `.bin` anteriores. Salvar com o sistema vazio apaga os dados gravados antes.
- Os textos têm tamanho máximo fixo e o que passar do limite é cortado.
- Os campos de texto não são validados quanto ao conteúdo (por exemplo, o formato da data e campos vazios).

## Apresentação e demonstração

- [Assistir à apresentação do trabalho no YouTube](https://youtu.be/3M8DGdiyDC0?si=sWQVPHNhADs8pApB)

## Apresentação e demonstração

- [Assistir à apresentação do trabalho no YouTube](SEU_LINK_DO_YOUTUBE_AQUI)
- [Acessar o repositório no GitHub](SEU_LINK_DO_REPOSITORIO_AQUI)