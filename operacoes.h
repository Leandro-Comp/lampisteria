#ifndef OPERACOES_H
#define OPERACOES_H

#define TAM_DESC_OP 160
#define TAM_DATA_OP 20
#define TAM_ID_REL 100

typedef struct NoOperacao {
    char descricao[TAM_DESC_OP];
    char data[TAM_DATA_OP];
    char identificadores[TAM_ID_REL];
    struct NoOperacao *prox;
} NoOperacao;

typedef struct {
    NoOperacao *topo;
    int quantidade;
} PilhaOperacoes;

void inicializar_pilha(PilhaOperacoes *pilha);
int pilha_vazia(const PilhaOperacoes *pilha);
int empilhar_operacao(PilhaOperacoes *pilha,
                      const char *descricao,
                      const char *data,
                      const char *identificadores);
void listar_operacoes(const PilhaOperacoes *pilha);
void liberar_pilha(PilhaOperacoes *pilha);

#endif