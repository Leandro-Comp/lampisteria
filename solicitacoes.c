#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "solicitacoes.h"

void inicializar_fila(FilaSolicitacoes *fila) {
    if (fila == NULL) {
        return;
    }
    fila->frente = NULL;
    fila->tras = NULL;
    fila->tamanho = 0;
    fila->proximo_protocolo = 1;
}

int fila_vazia(const FilaSolicitacoes *fila) {
    if (fila == NULL) {
        return 1;
    }
    return fila->frente == NULL;
}

int enfileirar_solicitacao(FilaSolicitacoes *fila,
                           const char *solicitante,
                           const char *matricula,
                           const char *categoria,
                           const char *local_trabalho,
                           const char *data,
                           int *protocolo_gerado) {
    if (fila == NULL) {
        return 0;
    }

    NoSolicitacao *novo = malloc(sizeof(NoSolicitacao));
    if (novo == NULL) {
        printf("Erro: falha ao alocar memoria para solicitacao.\n");
        return 0;
    }

    novo->protocolo = fila->proximo_protocolo;
    snprintf(novo->solicitante, TAM_SOLICITANTE, "%s", solicitante ? solicitante : "");
    snprintf(novo->matricula, TAM_MATRICULA_SOL, "%s", matricula ? matricula : "");
    snprintf(novo->categoria, TAM_CAT_SOL, "%s", categoria ? categoria : "");
    snprintf(novo->local_trabalho, TAM_LOCAL_TRABALHO, "%s", local_trabalho ? local_trabalho : "");
    snprintf(novo->data, TAM_DATA, "%s", data ? data : "");
    novo->prox = NULL;

    if (fila->tras == NULL) {
        fila->frente = novo;
        fila->tras = novo;
    } else {
        fila->tras->prox = novo;
        fila->tras = novo;
    }

    fila->tamanho++;
    if (protocolo_gerado != NULL) {
        *protocolo_gerado = novo->protocolo;
    }
    fila->proximo_protocolo++;

    return 1;
}

int reenfileirar_inicio(FilaSolicitacoes *fila, NoSolicitacao *no) {
    if (fila == NULL || no == NULL) {
        return 0;
    }

    no->prox = fila->frente;
    fila->frente = no;
    if (fila->tras == NULL) {
        fila->tras = no;
    }
    fila->tamanho++;
    return 1;
}

int reenfileirar_fim(FilaSolicitacoes *fila, NoSolicitacao *no) {
    if (fila == NULL || no == NULL) {
        return 0;
    }

    no->prox = NULL;
    if (fila->tras == NULL) {
        fila->frente = no;
        fila->tras = no;
    } else {
        fila->tras->prox = no;
        fila->tras = no;
    }
    fila->tamanho++;
    return 1;
}

NoSolicitacao *desenfileirar_no(FilaSolicitacoes *fila) {
    if (fila == NULL || fila->frente == NULL) {
        return NULL;
    }

    NoSolicitacao *removido = fila->frente;
    fila->frente = removido->prox;
    if (fila->frente == NULL) {
        fila->tras = NULL;
    }
    removido->prox = NULL;
    fila->tamanho--;
    return removido;
}

int desenfileirar_solicitacao(FilaSolicitacoes *fila, NoSolicitacao *removida) {
    NoSolicitacao *no = desenfileirar_no(fila);
    if (no == NULL) {
        return 0;
    }

    if (removida != NULL) {
        *removida = *no;
        removida->prox = NULL;
    }
    free(no);
    return 1;
}

int cancelar_solicitacao(FilaSolicitacoes *fila, int protocolo, NoSolicitacao *cancelada) {
    if (fila == NULL || fila->frente == NULL) {
        return 0;
    }

    NoSolicitacao *atual = fila->frente;
    NoSolicitacao *anterior = NULL;

    while (atual != NULL && atual->protocolo != protocolo) {
        anterior = atual;
        atual = atual->prox;
    }

    if (atual == NULL) {
        return 0;
    }

    if (anterior == NULL) {
        fila->frente = atual->prox;
    } else {
        anterior->prox = atual->prox;
    }

    if (atual == fila->tras) {
        fila->tras = anterior;
    }

    fila->tamanho--;
    if (cancelada != NULL) {
        *cancelada = *atual;
        cancelada->prox = NULL;
    }

    free(atual);
    return 1;
}

void listar_solicitacoes(const FilaSolicitacoes *fila) {
    if (fila == NULL || fila->frente == NULL) {
        printf("Fila de solicitacoes vazia.\n");
        return;
    }

    printf("Fila de solicitacoes (%d):\n", fila->tamanho);
    const NoSolicitacao *atual = fila->frente;
    int pos = 1;
    while (atual != NULL) {
        printf("%do | Protocolo %d | %s (%s) | %s | %s | %s\n",
               pos,
               atual->protocolo,
               atual->solicitante,
               atual->matricula,
               atual->categoria,
               atual->local_trabalho,
               atual->data);
        atual = atual->prox;
        pos++;
    }
}

void liberar_fila(FilaSolicitacoes *fila) {
    if (fila == NULL) {
        return;
    }

    NoSolicitacao *atual = fila->frente;
    while (atual != NULL) {
        NoSolicitacao *prox = atual->prox;
        free(atual);
        atual = prox;
    }

    fila->frente = NULL;
    fila->tras = NULL;
    fila->tamanho = 0;
}