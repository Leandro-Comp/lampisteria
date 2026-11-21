#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "operacoes.h"

void inicializar_pilha(PilhaOperacoes *pilha) {
    if (pilha == NULL) {
        return;
    }
    pilha->topo = NULL;
    pilha->quantidade = 0;
}

int pilha_vazia(const PilhaOperacoes *pilha) {
    if (pilha == NULL) {
        return 1;
    }
    return pilha->topo == NULL;
}

int empilhar_operacao(PilhaOperacoes *pilha,
                      const char *descricao,
                      const char *data,
                      const char *identificadores) {
    if (pilha == NULL) {
        return 0;
    }

    NoOperacao *novo = malloc(sizeof(NoOperacao));
    if (novo == NULL) {
        printf("Erro: falha ao alocar memoria para operacao recente.\n");
        return 0;
    }

    snprintf(novo->descricao, TAM_DESC_OP, "%s", descricao ? descricao : "");
    snprintf(novo->data, TAM_DATA_OP, "%s", data ? data : "");
    snprintf(novo->identificadores, TAM_ID_REL, "%s", identificadores ? identificadores : "-");
    novo->prox = pilha->topo;

    pilha->topo = novo;
    pilha->quantidade++;
    return 1;
}

void listar_operacoes(const PilhaOperacoes *pilha) {
    if (pilha == NULL || pilha->topo == NULL) {
        printf("Nenhuma operacao recente registrada.\n");
        return;
    }

    const NoOperacao *atual = pilha->topo;
    int ordem = pilha->quantidade;

    while (atual != NULL) {
        if (atual->identificadores[0] != '\0' && strcmp(atual->identificadores, "-") != 0) {
            if (atual->data[0] != '\0') {
                printf("%d. %s [%s] (%s)\n",
                       ordem, atual->descricao, atual->identificadores, atual->data);
            } else {
                printf("%d. %s [%s]\n",
                       ordem, atual->descricao, atual->identificadores);
            }
        } else {
            if (atual->data[0] != '\0') {
                printf("%d. %s (%s)\n",
                       ordem, atual->descricao, atual->data);
            } else {
                printf("%d. %s\n", ordem, atual->descricao);
            }
        }
        atual = atual->prox;
        ordem--;
    }
}

void liberar_pilha(PilhaOperacoes *pilha) {
    if (pilha == NULL) {
        return;
    }

    NoOperacao *atual = pilha->topo;
    while (atual != NULL) {
        NoOperacao *prox = atual->prox;
        free(atual);
        atual = prox;
    }

    pilha->topo = NULL;
    pilha->quantidade = 0;
}