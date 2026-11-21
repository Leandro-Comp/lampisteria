#ifndef HISTORICO_H
#define HISTORICO_H

#define TAM_TIPO_MOV 40
#define TAM_DETALHE_MOV 200
#define TAM_DATA_MOV 20

typedef struct NoHistorico {
    int sequencia;
    char tipo[TAM_TIPO_MOV];
    char detalhe[TAM_DETALHE_MOV];
    char data[TAM_DATA_MOV];
    struct NoHistorico *prox;
} NoHistorico;

typedef struct {
    NoHistorico *inicio;
    NoHistorico *fim;
    int total;
} ListaHistorico;

void inicializar_historico(ListaHistorico *historico);
int registrar_historico(ListaHistorico *historico,
                        const char *tipo,
                        const char *detalhe,
                        const char *data);
void listar_historico(const ListaHistorico *historico);
void liberar_historico(ListaHistorico *historico);

#endif