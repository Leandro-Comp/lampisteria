#ifndef EQUIPAMENTOS_H
#define EQUIPAMENTOS_H

#define TAM_DESCRICAO 100
#define TAM_CATEGORIA 50
#define TAM_PATRIMONIO 40
#define TAM_NOME 100
#define TAM_MATRICULA 30

typedef enum {
    ESTADO_DISPONIVEL = 0,
    ESTADO_EMPRESTADO = 1,
    ESTADO_MANUTENCAO = 2
} EstadoEquipamento;

typedef struct Equipamento {
    int id;
    char descricao[TAM_DESCRICAO];
    char categoria[TAM_CATEGORIA];
    char patrimonio[TAM_PATRIMONIO];
    EstadoEquipamento estado;
    char responsavel_nome[TAM_NOME];
    char responsavel_matricula[TAM_MATRICULA];
    struct Equipamento *prox;
} Equipamento;

int inserir_equipamento(Equipamento **inicio, Equipamento dados);
Equipamento *buscar_equipamento(Equipamento *inicio, int id);
Equipamento *buscar_primeiro_disponivel(Equipamento *inicio, const char *categoria);
int usuario_possui_categoria_emprestada(Equipamento *inicio, const char *matricula, const char *categoria);
void listar_equipamentos(const Equipamento *inicio);
void listar_equipamentos_por_categoria(const Equipamento *inicio, const char *categoria);
void liberar_equipamentos(Equipamento **inicio);
const char *estado_para_texto(EstadoEquipamento estado);

#endif