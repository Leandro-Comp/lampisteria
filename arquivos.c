#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arquivos.h"

typedef struct {
    int id;
    char descricao[TAM_DESCRICAO];
    char categoria[TAM_CATEGORIA];
    char patrimonio[TAM_PATRIMONIO];
    EstadoEquipamento estado;
    char responsavel_nome[TAM_NOME];
    char responsavel_matricula[TAM_MATRICULA];
} RegistroEquipamentoBin;

typedef struct {
    int sequencia;
    char tipo[TAM_TIPO_MOV];
    char detalhe[TAM_DETALHE_MOV];
    char data[TAM_DATA_MOV];
} RegistroHistoricoBin;

int salvar_equipamentos_bin(const char *caminho, const Equipamento *inicio, int *total_salvos) {
    if (caminho == NULL || total_salvos == NULL) {
        return 0;
    }

    *total_salvos = 0;
    FILE *fp = fopen(caminho, "wb");
    if (fp == NULL) {
        printf("Erro: nao foi possivel abrir '%s' para escrita.\n", caminho);
        return 0;
    }

    int contagem = 0;
    const Equipamento *atual = inicio;
    while (atual != NULL) {
        contagem++;
        atual = atual->prox;
    }

    if (fwrite(&contagem, sizeof(int), 1, fp) != 1) {
        printf("Erro: falha ao gravar quantidade de equipamentos em '%s'.\n", caminho);
        fclose(fp);
        return 0;
    }

    atual = inicio;
    while (atual != NULL) {
        RegistroEquipamentoBin reg;
        memset(&reg, 0, sizeof(reg));
        reg.id = atual->id;
        snprintf(reg.descricao, TAM_DESCRICAO, "%s", atual->descricao);
        snprintf(reg.categoria, TAM_CATEGORIA, "%s", atual->categoria);
        snprintf(reg.patrimonio, TAM_PATRIMONIO, "%s", atual->patrimonio);
        reg.estado = atual->estado;
        snprintf(reg.responsavel_nome, TAM_NOME, "%s", atual->responsavel_nome);
        snprintf(reg.responsavel_matricula, TAM_MATRICULA, "%s", atual->responsavel_matricula);

        if (fwrite(&reg, sizeof(RegistroEquipamentoBin), 1, fp) != 1) {
            printf("Erro: falha ao gravar equipamento ID %d em '%s'.\n", atual->id, caminho);
            fclose(fp);
            return 0;
        }

        (*total_salvos)++;
        atual = atual->prox;
    }

    if (fclose(fp) != 0) {
        printf("Erro: falha ao fechar '%s'.\n", caminho);
        return 0;
    }

    return 1;
}

int carregar_equipamentos_bin(const char *caminho, Equipamento **inicio, int *total_carregados) {
    if (caminho == NULL || inicio == NULL || total_carregados == NULL) {
        return 0;
    }

    *total_carregados = 0;
    FILE *fp = fopen(caminho, "rb");
    if (fp == NULL) {
        printf("Erro: nao foi possivel abrir '%s' para leitura.\n", caminho);
        return 0;
    }

    int contagem = 0;
    if (fread(&contagem, sizeof(int), 1, fp) != 1 || contagem < 0) {
        printf("Erro: arquivo '%s' invalido ou corrompido.\n", caminho);
        fclose(fp);
        return 0;
    }

    Equipamento *nova_lista = NULL;
    Equipamento *ultimo = NULL;

    for (int i = 0; i < contagem; i++) {
        RegistroEquipamentoBin reg;
        if (fread(&reg, sizeof(RegistroEquipamentoBin), 1, fp) != 1) {
            printf("Erro: leitura incompleta dos equipamentos em '%s'.\n", caminho);
            liberar_equipamentos(&nova_lista);
            fclose(fp);
            return 0;
        }

        Equipamento *no = malloc(sizeof(Equipamento));
        if (no == NULL) {
            printf("Erro: falha ao alocar memoria ao carregar equipamento.\n");
            liberar_equipamentos(&nova_lista);
            fclose(fp);
            return 0;
        }

        no->id = reg.id;
        snprintf(no->descricao, TAM_DESCRICAO, "%s", reg.descricao);
        snprintf(no->categoria, TAM_CATEGORIA, "%s", reg.categoria);
        snprintf(no->patrimonio, TAM_PATRIMONIO, "%s", reg.patrimonio);
        no->estado = reg.estado;
        snprintf(no->responsavel_nome, TAM_NOME, "%s", reg.responsavel_nome);
        snprintf(no->responsavel_matricula, TAM_MATRICULA, "%s", reg.responsavel_matricula);
        no->prox = NULL;

        if (nova_lista == NULL) {
            nova_lista = no;
            ultimo = no;
        } else {
            ultimo->prox = no;
            ultimo = no;
        }

        (*total_carregados)++;
    }

    fclose(fp);
    liberar_equipamentos(inicio);
    *inicio = nova_lista;
    return 1;
}

int salvar_historico_bin(const char *caminho, const ListaHistorico *historico, int *total_salvos) {
    if (caminho == NULL || historico == NULL || total_salvos == NULL) {
        return 0;
    }

    *total_salvos = 0;
    FILE *fp = fopen(caminho, "wb");
    if (fp == NULL) {
        printf("Erro: nao foi possivel abrir '%s' para escrita.\n", caminho);
        return 0;
    }

    int contagem = historico->total;
    if (fwrite(&contagem, sizeof(int), 1, fp) != 1) {
        printf("Erro: falha ao gravar quantidade de movimentacoes em '%s'.\n", caminho);
        fclose(fp);
        return 0;
    }

    const NoHistorico *atual = historico->inicio;
    while (atual != NULL) {
        RegistroHistoricoBin reg;
        memset(&reg, 0, sizeof(reg));
        reg.sequencia = atual->sequencia;
        snprintf(reg.tipo, TAM_TIPO_MOV, "%s", atual->tipo);
        snprintf(reg.detalhe, TAM_DETALHE_MOV, "%s", atual->detalhe);
        snprintf(reg.data, TAM_DATA_MOV, "%s", atual->data);

        if (fwrite(&reg, sizeof(RegistroHistoricoBin), 1, fp) != 1) {
            printf("Erro: falha ao gravar movimentacao em '%s'.\n", caminho);
            fclose(fp);
            return 0;
        }

        (*total_salvos)++;
        atual = atual->prox;
    }

    if (fclose(fp) != 0) {
        printf("Erro: falha ao fechar '%s'.\n", caminho);
        return 0;
    }

    return 1;
}

int carregar_historico_bin(const char *caminho, ListaHistorico *historico, int *total_carregados) {
    if (caminho == NULL || historico == NULL || total_carregados == NULL) {
        return 0;
    }

    *total_carregados = 0;
    FILE *fp = fopen(caminho, "rb");
    if (fp == NULL) {
        printf("Erro: nao foi possivel abrir '%s' para leitura.\n", caminho);
        return 0;
    }

    int contagem = 0;
    if (fread(&contagem, sizeof(int), 1, fp) != 1 || contagem < 0) {
        printf("Erro: arquivo '%s' invalido ou corrompido.\n", caminho);
        fclose(fp);
        return 0;
    }

    ListaHistorico novo_hist;
    inicializar_historico(&novo_hist);

    for (int i = 0; i < contagem; i++) {
        RegistroHistoricoBin reg;
        if (fread(&reg, sizeof(RegistroHistoricoBin), 1, fp) != 1) {
            printf("Erro: leitura incompleta do historico em '%s'.\n", caminho);
            liberar_historico(&novo_hist);
            fclose(fp);
            return 0;
        }

        NoHistorico *no = malloc(sizeof(NoHistorico));
        if (no == NULL) {
            printf("Erro: falha ao alocar memoria ao carregar historico.\n");
            liberar_historico(&novo_hist);
            fclose(fp);
            return 0;
        }

        no->sequencia = reg.sequencia;
        snprintf(no->tipo, TAM_TIPO_MOV, "%s", reg.tipo);
        snprintf(no->detalhe, TAM_DETALHE_MOV, "%s", reg.detalhe);
        snprintf(no->data, TAM_DATA_MOV, "%s", reg.data);
        no->prox = NULL;

        if (novo_hist.fim == NULL) {
            novo_hist.inicio = no;
            novo_hist.fim = no;
        } else {
            novo_hist.fim->prox = no;
            novo_hist.fim = no;
        }
        novo_hist.total++;
        (*total_carregados)++;
    }

    fclose(fp);
    liberar_historico(historico);
    *historico = novo_hist;
    return 1;
}