#ifndef SOLICITACOES_H
#define SOLICITACOES_H

#define TAM_SOLICITANTE 100
#define TAM_MATRICULA_SOL 30
#define TAM_CAT_SOL 50
#define TAM_LOCAL_TRABALHO 100
#define TAM_DATA 20

typedef struct NoSolicitacao {
    int protocolo;
    char solicitante[TAM_SOLICITANTE];
    char matricula[TAM_MATRICULA_SOL];
    char categoria[TAM_CAT_SOL];
    char local_trabalho[TAM_LOCAL_TRABALHO];
    char data[TAM_DATA];
    struct NoSolicitacao *prox;
} NoSolicitacao;

typedef struct {
    NoSolicitacao *frente;
    NoSolicitacao *tras;
    int tamanho;
    int proximo_protocolo;
} FilaSolicitacoes;

void inicializar_fila(FilaSolicitacoes *fila);
int fila_vazia(const FilaSolicitacoes *fila);
int enfileirar_solicitacao(FilaSolicitacoes *fila,
                           const char *solicitante,
                           const char *matricula,
                           const char *categoria,
                           const char *local_trabalho,
                           const char *data,
                           int *protocolo_gerado);
int reenfileirar_inicio(FilaSolicitacoes *fila, NoSolicitacao *no);
int reenfileirar_fim(FilaSolicitacoes *fila, NoSolicitacao *no);
NoSolicitacao *desenfileirar_no(FilaSolicitacoes *fila);
int desenfileirar_solicitacao(FilaSolicitacoes *fila, NoSolicitacao *removida);
int cancelar_solicitacao(FilaSolicitacoes *fila, int protocolo, NoSolicitacao *cancelada);
void listar_solicitacoes(const FilaSolicitacoes *fila);
void liberar_fila(FilaSolicitacoes *fila);

#endif