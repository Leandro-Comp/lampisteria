# Controle da Lampisteria

Trabalho 2 da disciplina de **Introdução à Programação de Computadores** (CEFET-MG — Divinópolis, Engenharia de Computação), ministrada pelo Prof. Guido Pantuza.

O sistema gerencia o empréstimo, devolução, manutenção e controle de equipamentos da lampisteria (como lanternas, autorresgatadores, detectores de gás e rádios), organizando as solicitações dos mineiros em fila, registrando as operações recentes e o histórico das movimentações, além de salvar e carregar os dados em arquivos binários.

## Estrutura do projeto

Separei o código em módulos `.h` e `.c` para manter cada parte com sua responsabilidade:

- `main.c`: loop do menu principal (opções `0` a `12`), leitura de entradas com `fgets` e chamadas para os módulos.
- `equipamentos.h` / `equipamentos.c`: lista encadeada dinâmica de equipamentos mantida em ordem crescente de `id`, com verificação de ID repetido, busca, listagem e liberação de memória.
- `solicitacoes.h` / `solicitacoes.c`: fila dinâmica (FIFO) de solicitações com numeração sequencial de protocolo a partir de `1`, atendimento pela ordem de chegada, recolocação no início ou fim quando não há item disponível e cancelamento por protocolo sem alterar a ordem dos demais pedidos.
- `operacoes.h` / `operacoes.c`: pilha dinâmica para as operações concluídas com sucesso, exibindo sempre a mais recente no topo.
- `historico.h` / `historico.c`: lista simplesmente encadeada em ordem cronológica para registrar as movimentações concluídas (empréstimo, devolução, entrada/saída de manutenção e cancelamento).
- `arquivos.h` / `arquivos.c`: gravação e leitura em arquivos binários (`equipamentos.bin` e `historico.bin`) usando `fwrite` e `fread`.

## Decisões de projeto

- **Lista ordenada por ID**: escolhi inserir os equipamentos já ordenados por identificador na lista encadeada para facilitar a busca por ID e garantir que o atendimento pegue o primeiro equipamento disponível da categoria pedida na ordem crescente de ID.
- **Arquivos binários sem salvar ponteiros**: na persistência em arquivo, gravo apenas os campos de dados dos registros e reconstruo os nós dinamicamente ao carregar, depois de liberar o que estava em memória, evitando endereços de memória inválidos entre execuções.
- **Apenas operações válidas na pilha**: resolvi empilhar somente as ações concluídas com êxito para que tentativas inválidas (como ID repetido, equipamento inexistente, devolução de item não emprestado ou protocolo inexistente) não entrem na pilha de operações recentes.

## Como compilar e rodar

No terminal, dentro da pasta do projeto:

```bash
gcc -Wall -Wextra main.c equipamentos.c solicitacoes.c operacoes.c historico.c arquivos.c -o lampisteria.exe

Para executar:
./lampisteria