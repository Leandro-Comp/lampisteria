#ifndef ARQUIVOS_H
#define ARQUIVOS_H

#include "equipamentos.h"
#include "historico.h"

#define ARQUIVO_EQUIPAMENTOS "equipamentos.bin"
#define ARQUIVO_HISTORICO "historico.bin"

int salvar_equipamentos_bin(const char *caminho, const Equipamento *inicio, int *total_salvos);
int carregar_equipamentos_bin(const char *caminho, Equipamento **inicio, int *total_carregados);

int salvar_historico_bin(const char *caminho, const ListaHistorico *historico, int *total_salvos);
int carregar_historico_bin(const char *caminho, ListaHistorico *historico, int *total_carregados);

#endif