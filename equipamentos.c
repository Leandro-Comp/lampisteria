#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "equipamentos.h"

const char *estado_para_texto(EstadoEquipamento estado) {
    switch (estado) {
        case ESTADO_DISPONIVEL:
            return "Disponivel";
        case ESTADO_EMPRESTADO:
            return "Emprestado";
        case ESTADO_MANUTENCAO:
            return "Manutencao";
        default:
            return "Desconhecido";
    }
}

int inserir_equipamento(Equipamento **inicio, Equipamento dados) {
    if (inicio == NULL) {
        return 0;
    }

    Equipamento *atual = *inicio;
    Equipamento *anterior = NULL;

    while (atual != NULL && atual->id < dados.id) {
        anterior = atual;
        atual = atual->prox;
    }

    if (atual != NULL && atual->id == dados.id) {
        return 0;
    }

    Equipamento *novo = malloc(sizeof(Equipamento));
    if (novo == NULL) {
        printf("Erro: falha ao alocar memoria para equipamento.\n");
        return 0;
    }

    *novo = dados;
    novo->estado = ESTADO_DISPONIVEL;
    novo->responsavel_nome[0] = '\0';
    novo->responsavel_matricula[0] = '\0';
    novo->prox = atual;

    if (anterior == NULL) {
        *inicio = novo;
    } else {
        anterior->prox = novo;
    }

    return 1;
}

Equipamento *buscar_equipamento(Equipamento *inicio, int id) {
    Equipamento *atual = inicio;
    while (atual != NULL) {
        if (atual->id == id) {
            return atual;
        }
        if (atual->id > id) {
            break;
        }
        atual = atual->prox;
    }
    return NULL;
}

Equipamento *buscar_primeiro_disponivel(Equipamento *inicio, const char *categoria) {
    Equipamento *atual = inicio;
    while (atual != NULL) {
        if (atual->estado == ESTADO_DISPONIVEL &&
            strcmp(atual->categoria, categoria) == 0) {
            return atual;
        }
        atual = atual->prox;
    }
    return NULL;
}

int usuario_possui_categoria_emprestada(Equipamento *inicio, const char *matricula, const char *categoria) {
    Equipamento *atual = inicio;
    while (atual != NULL) {
        if (atual->estado == ESTADO_EMPRESTADO &&
            strcmp(atual->responsavel_matricula, matricula) == 0 &&
            strcmp(atual->categoria, categoria) == 0) {
            return 1;
        }
        atual = atual->prox;
    }
    return 0;
}

void listar_equipamentos(const Equipamento *inicio) {
    if (inicio == NULL) {
        printf("Nenhum equipamento cadastrado.\n");
        return;
    }

    printf("%-6s %-26s %-18s %-14s %-12s %s\n",
           "ID", "Descricao", "Categoria", "Patrimonio", "Estado", "Responsavel");

    const Equipamento *atual = inicio;
    while (atual != NULL) {
        if (atual->estado == ESTADO_EMPRESTADO && atual->responsavel_nome[0] != '\0') {
            printf("%-6d %-26s %-18s %-14s %-12s %s (%s)\n",
                   atual->id,
                   atual->descricao,
                   atual->categoria,
                   atual->patrimonio,
                   estado_para_texto(atual->estado),
                   atual->responsavel_nome,
                   atual->responsavel_matricula);
        } else {
            printf("%-6d %-26s %-18s %-14s %-12s -\n",
                   atual->id,
                   atual->descricao,
                   atual->categoria,
                   atual->patrimonio,
                   estado_para_texto(atual->estado));
        }
        atual = atual->prox;
    }
}

void listar_equipamentos_por_categoria(const Equipamento *inicio, const char *categoria) {
    int encontrados = 0;
    const Equipamento *atual = inicio;

    printf("\n--- Equipamentos da categoria '%s' ---\n", categoria);
    while (atual != NULL) {
        if (strcmp(atual->categoria, categoria) == 0) {
            if (atual->estado == ESTADO_EMPRESTADO && atual->responsavel_nome[0] != '\0') {
                printf("[%d] %s | Pat: %s | %s | Resp: %s (%s)\n",
                       atual->id,
                       atual->descricao,
                       atual->patrimonio,
                       estado_para_texto(atual->estado),
                       atual->responsavel_nome,
                       atual->responsavel_matricula);
            } else {
                printf("[%d] %s | Pat: %s | %s\n",
                       atual->id,
                       atual->descricao,
                       atual->patrimonio,
                       estado_para_texto(atual->estado));
            }
            encontrados++;
        }
        atual = atual->prox;
    }

    if (encontrados == 0) {
        printf("Nenhum equipamento encontrado nessa categoria.\n");
    }
}

void liberar_equipamentos(Equipamento **inicio) {
    if (inicio == NULL) {
        return;
    }

    Equipamento *atual = *inicio;
    while (atual != NULL) {
        Equipamento *prox = atual->prox;
        free(atual);
        atual = prox;
    }

    *inicio = NULL;
}