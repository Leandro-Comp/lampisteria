#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "historico.h"

void inicializar_historico(ListaHistorico *historico) {
    if (historico == NULL) {
        return;
    }
    historico->inicio = NULL;
    historico->fim = NULL;
    historico->total = 0;
}

int registrar_historico(ListaHistorico *historico,
                        const char *tipo,
                        const char *detalhe,
                        const char *data) {
    if (historico == NULL) {
        return 0;
    }

    NoHistorico *novo = malloc(sizeof(NoHistorico));
    if (novo == NULL) {
        printf("Erro: falha ao alocar memoria para o historico.\n");
        return 0;
    }

    historico->total++;
    novo->sequencia = historico->total;
    snprintf(novo->tipo, TAM_TIPO_MOV, "%s", tipo ? tipo : "");
    snprintf(novo->detalhe, TAM_DETALHE_MOV, "%s", detalhe ? detalhe : "");
    snprintf(novo->data, TAM_DATA_MOV, "%s", data ? data : "");
    novo->prox = NULL;

    if (historico->fim == NULL) {
        historico->inicio = novo;
        historico->fim = novo;
    } else {
        historico->fim->prox = novo;
        historico->fim = novo;
    }

    return 1;
}

void listar_historico(const ListaHistorico *historico) {
    if (historico == NULL || historico->inicio == NULL) {
        printf("Nenhuma movimentacao registrada no historico.\n");
        return;
    }

    printf("Historico de movimentacoes (%d):\n", historico->total);
    const NoHistorico *atual = historico->inicio;
    while (atual != NULL) {
        if (atual->data[0] != '\0') {
            printf("%d. %s | %s (%s)\n",
                   atual->sequencia, atual->tipo, atual->detalhe, atual->data);
        } else {
            printf("%d. %s | %s\n",
                   atual->sequencia, atual->tipo, atual->detalhe);
        }
        atual = atual->prox;
    }
}

void liberar_historico(ListaHistorico *historico) {
    if (historico == NULL) {
        return;
    }

    NoHistorico *atual = historico->inicio;
    while (atual != NULL) {
        NoHistorico *prox = atual->prox;
        free(atual);
        atual = prox;
    }

    historico->inicio = NULL;
    historico->fim = NULL;
    historico->total = 0;
}