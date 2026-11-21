#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "equipamentos.h"
#include "solicitacoes.h"
#include "operacoes.h"
#include "historico.h"
#include "arquivos.h"

/* Passa a 1 quando a entrada padrao acaba (EOF), para evitar lacos infinitos */
static int fim_entrada = 0;

static void limpar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

static void ler_linha(const char *prompt, char *destino, size_t tamanho) {
    if (destino == NULL || tamanho == 0) {
        return;
    }

    if (prompt != NULL) {
        printf("%s", prompt);
    }

    if (fgets(destino, (int)tamanho, stdin) == NULL) {
        fim_entrada = 1;
        destino[0] = '\0';
        return;
    }

    size_t len = strlen(destino);
    if (len > 0 && destino[len - 1] == '\n') {
        destino[len - 1] = '\0';
    }
}

static int ler_inteiro(const char *prompt, int *valor) {
    if (valor == NULL) {
        return 0;
    }

    if (prompt != NULL) {
        printf("%s", prompt);
    }

    int r = scanf("%d", valor);
    if (r == EOF) {
        fim_entrada = 1;
        return 0;
    }
    if (r != 1) {
        printf("Erro: entrada invalida. Digite um numero inteiro.\n");
        limpar_buffer();
        return 0;
    }

    limpar_buffer();
    return 1;
}

static void opcao_cadastrar_equipamento(Equipamento **lista_eq, PilhaOperacoes *pilha_op) {
    Equipamento novo;
    memset(&novo, 0, sizeof(novo));

    if (!ler_inteiro("ID do equipamento: ", &novo.id)) {
        return;
    }

    /* Le todos os campos antes de validar, para nao dessincronizar a entrada */
    ler_linha("Descricao: ", novo.descricao, sizeof(novo.descricao));
    ler_linha("Categoria (ex: Lanterna, Autorresgatador, Detector de gas, Radio): ",
              novo.categoria, sizeof(novo.categoria));
    ler_linha("Codigo de patrimonio: ", novo.patrimonio, sizeof(novo.patrimonio));

    if (buscar_equipamento(*lista_eq, novo.id) != NULL) {
        printf("Erro: ja existe equipamento com identificador %d.\n", novo.id);
        return;
    }

    if (!inserir_equipamento(lista_eq, novo)) {
        printf("Erro: nao foi possivel cadastrar o equipamento %d.\n", novo.id);
        return;
    }

    char desc_op[TAM_DESC_OP];
    char id_rel[TAM_ID_REL];
    snprintf(desc_op, sizeof(desc_op), "Cadastro do equipamento %d (%s)", novo.id, novo.categoria);
    snprintf(id_rel, sizeof(id_rel), "Equipamento %d", novo.id);
    empilhar_operacao(pilha_op, desc_op, "", id_rel);

    printf("Equipamento %d cadastrado.\n", novo.id);
}

static void opcao_buscar_equipamento(Equipamento *lista_eq) {
    int tipo_busca = 0;
    printf("Buscar por: (1) ID  (2) Categoria\n");
    if (!ler_inteiro("Escolha: ", &tipo_busca)) {
        return;
    }

    if (tipo_busca == 1) {
        int id = 0;
        if (!ler_inteiro("Informe o ID: ", &id)) {
            return;
        }

        Equipamento *eq = buscar_equipamento(lista_eq, id);
        if (eq == NULL) {
            printf("Erro: equipamento %d nao encontrado.\n", id);
            return;
        }

        printf("ID: %d | Descricao: %s | Categoria: %s | Patrimonio: %s | Estado: %s",
               eq->id, eq->descricao, eq->categoria, eq->patrimonio, estado_para_texto(eq->estado));
        if (eq->estado == ESTADO_EMPRESTADO && eq->responsavel_nome[0] != '\0') {
            printf(" | Responsavel: %s (%s)", eq->responsavel_nome, eq->responsavel_matricula);
        }
        printf("\n");
    } else if (tipo_busca == 2) {
        char categoria[TAM_CATEGORIA];
        ler_linha("Informe a categoria: ", categoria, sizeof(categoria));
        listar_equipamentos_por_categoria(lista_eq, categoria);
    } else {
        printf("Opcao de busca invalida.\n");
    }
}

static void opcao_registrar_solicitacao(FilaSolicitacoes *fila,
                                        Equipamento *lista_eq,
                                        PilhaOperacoes *pilha_op) {
    char solicitante[TAM_SOLICITANTE];
    char matricula[TAM_MATRICULA_SOL];
    char categoria[TAM_CAT_SOL];
    char local_trabalho[TAM_LOCAL_TRABALHO];
    char data[TAM_DATA];

    ler_linha("Nome do solicitante: ", solicitante, sizeof(solicitante));
    ler_linha("Matricula: ", matricula, sizeof(matricula));
    ler_linha("Categoria solicitada: ", categoria, sizeof(categoria));
    ler_linha("Local de trabalho no turno (frente de lavra, galeria ou nivel): ",
              local_trabalho, sizeof(local_trabalho));
    ler_linha("Data (ex: 05/10/2026): ", data, sizeof(data));

    if (usuario_possui_categoria_emprestada(lista_eq, matricula, categoria)) {
        printf("Erro: %s (%s) ja possui um %s emprestado.\n",
               solicitante, matricula, categoria);
        return;
    }

    int protocolo = 0;
    if (!enfileirar_solicitacao(fila, solicitante, matricula, categoria,
                                local_trabalho, data, &protocolo)) {
        return;
    }

    char desc_op[TAM_DESC_OP];
    char id_rel[TAM_ID_REL];
    snprintf(desc_op, sizeof(desc_op), "Solicitacao registrada: protocolo %d (%s)",
             protocolo, solicitante);
    snprintf(id_rel, sizeof(id_rel), "Protocolo %d | Matricula %s", protocolo, matricula);
    empilhar_operacao(pilha_op, desc_op, data, id_rel);

    printf("Solicitacao registrada. Protocolo: %d. Posicao na fila: %d.\n",
           protocolo, fila->tamanho);
}

static void opcao_atender_solicitacao(FilaSolicitacoes *fila,
                                      Equipamento *lista_eq,
                                      PilhaOperacoes *pilha_op,
                                      ListaHistorico *historico) {
    if (fila_vazia(fila)) {
        printf("Fila de solicitacoes vazia.\n");
        return;
    }

    NoSolicitacao *no_sol = desenfileirar_no(fila);
    if (no_sol == NULL) {
        printf("Fila de solicitacoes vazia.\n");
        return;
    }

    /* Mesma regra do registro: a matricula pode ter entrado na fila duas vezes
       para a mesma categoria. Decisao de projeto: a solicitacao e descartada. */
    if (usuario_possui_categoria_emprestada(lista_eq, no_sol->matricula, no_sol->categoria)) {
        printf("Erro: %s (%s) ja possui um %s emprestado. Solicitacao %d descartada.\n",
               no_sol->solicitante, no_sol->matricula, no_sol->categoria, no_sol->protocolo);
        free(no_sol);
        return;
    }

    Equipamento *eq = buscar_primeiro_disponivel(lista_eq, no_sol->categoria);
    if (eq != NULL) {
        eq->estado = ESTADO_EMPRESTADO;
        snprintf(eq->responsavel_nome, sizeof(eq->responsavel_nome), "%s", no_sol->solicitante);
        snprintf(eq->responsavel_matricula, sizeof(eq->responsavel_matricula), "%s", no_sol->matricula);

        char detalhe_hist[TAM_DETALHE_MOV];
        snprintf(detalhe_hist, sizeof(detalhe_hist),
                 "equipamento %d | %s (%s) | protocolo %d",
                 eq->id, no_sol->solicitante, no_sol->matricula, no_sol->protocolo);
        registrar_historico(historico, "Emprestimo", detalhe_hist, no_sol->data);

        char desc_op[TAM_DESC_OP];
        char id_rel[TAM_ID_REL];
        snprintf(desc_op, sizeof(desc_op),
                 "Emprestimo do equipamento %d a %s (protocolo %d)",
                 eq->id, no_sol->solicitante, no_sol->protocolo);
        snprintf(id_rel, sizeof(id_rel), "Equipamento %d | Protocolo %d",
                 eq->id, no_sol->protocolo);
        empilhar_operacao(pilha_op, desc_op, no_sol->data, id_rel);

        printf("Protocolo %d (%s): equipamento %d emprestado.\n",
               no_sol->protocolo, no_sol->solicitante, eq->id);
        free(no_sol);
        return;
    }

    printf("Protocolo %d (%s): nao ha %s disponivel.\n",
           no_sol->protocolo, no_sol->solicitante, no_sol->categoria);

    int acao = 0;
    while (acao < 1 || acao > 3) {
        if (!ler_inteiro("(1) manter no inicio  (2) recolocar no final  (3) cancelar : ", &acao)) {
            /* Em EOF, mantem no inicio e sai do laco */
            acao = fim_entrada ? 1 : 0;
        }
    }

    if (acao == 1) {
        reenfileirar_inicio(fila, no_sol);

        char desc_op[TAM_DESC_OP];
        char id_rel[TAM_ID_REL];
        snprintf(desc_op, sizeof(desc_op),
                 "Protocolo %d mantido no inicio da fila", no_sol->protocolo);
        snprintf(id_rel, sizeof(id_rel), "Protocolo %d", no_sol->protocolo);
        empilhar_operacao(pilha_op, desc_op, no_sol->data, id_rel);

        printf("Protocolo %d mantido no inicio da fila.\n", no_sol->protocolo);
    } else if (acao == 2) {
        reenfileirar_fim(fila, no_sol);

        char desc_op[TAM_DESC_OP];
        char id_rel[TAM_ID_REL];
        snprintf(desc_op, sizeof(desc_op),
                 "Protocolo %d recolocado no final da fila", no_sol->protocolo);
        snprintf(id_rel, sizeof(id_rel), "Protocolo %d", no_sol->protocolo);
        empilhar_operacao(pilha_op, desc_op, no_sol->data, id_rel);

        printf("Protocolo %d recolocado no final da fila.\n", no_sol->protocolo);
    } else {
        char detalhe_hist[TAM_DETALHE_MOV];
        snprintf(detalhe_hist, sizeof(detalhe_hist),
                 "protocolo %d | %s (%s)",
                 no_sol->protocolo, no_sol->solicitante, no_sol->matricula);
        registrar_historico(historico, "Cancelamento", detalhe_hist, no_sol->data);

        char desc_op[TAM_DESC_OP];
        char id_rel[TAM_ID_REL];
        snprintf(desc_op, sizeof(desc_op),
                 "Cancelamento do protocolo %d (%s)",
                 no_sol->protocolo, no_sol->solicitante);
        snprintf(id_rel, sizeof(id_rel), "Protocolo %d", no_sol->protocolo);
        empilhar_operacao(pilha_op, desc_op, no_sol->data, id_rel);

        printf("Protocolo %d cancelado.\n", no_sol->protocolo);
        free(no_sol);
    }
}

static void opcao_registrar_devolucao(Equipamento *lista_eq,
                                      PilhaOperacoes *pilha_op,
                                      ListaHistorico *historico) {
    int id = 0;
    if (!ler_inteiro("ID do equipamento para devolucao: ", &id)) {
        return;
    }

    Equipamento *eq = buscar_equipamento(lista_eq, id);
    if (eq == NULL) {
        printf("Erro: equipamento %d nao encontrado.\n", id);
        return;
    }

    if (eq->estado != ESTADO_EMPRESTADO) {
        printf("Erro: equipamento %d nao esta emprestado.\n", id);
        return;
    }

    char resp_nome[TAM_NOME];
    char resp_mat[TAM_MATRICULA];
    snprintf(resp_nome, sizeof(resp_nome), "%s", eq->responsavel_nome);
    snprintf(resp_mat, sizeof(resp_mat), "%s", eq->responsavel_matricula);

    eq->estado = ESTADO_DISPONIVEL;
    eq->responsavel_nome[0] = '\0';
    eq->responsavel_matricula[0] = '\0';

    char detalhe_hist[TAM_DETALHE_MOV];
    snprintf(detalhe_hist, sizeof(detalhe_hist),
             "equipamento %d | %s (%s)", eq->id, resp_nome, resp_mat);
    registrar_historico(historico, "Devolucao", detalhe_hist, "");

    char desc_op[TAM_DESC_OP];
    char id_rel[TAM_ID_REL];
    snprintf(desc_op, sizeof(desc_op),
             "Devolucao do equipamento %d (%s)", eq->id, resp_nome);
    snprintf(id_rel, sizeof(id_rel), "Equipamento %d", eq->id);
    empilhar_operacao(pilha_op, desc_op, "", id_rel);

    printf("Devolucao registrada. Equipamento %d disponivel.\n", eq->id);
}

static void opcao_alterar_manutencao(Equipamento *lista_eq,
                                     PilhaOperacoes *pilha_op,
                                     ListaHistorico *historico) {
    int id = 0;
    if (!ler_inteiro("ID do equipamento: ", &id)) {
        return;
    }

    Equipamento *eq = buscar_equipamento(lista_eq, id);
    if (eq == NULL) {
        printf("Erro: equipamento %d nao encontrado.\n", id);
        return;
    }

    if (eq->estado == ESTADO_EMPRESTADO) {
        printf("Erro: equipamento %d esta emprestado e nao pode entrar em manutencao.\n", id);
        return;
    }

    if (eq->estado == ESTADO_DISPONIVEL) {
        eq->estado = ESTADO_MANUTENCAO;

        char detalhe_hist[TAM_DETALHE_MOV];
        snprintf(detalhe_hist, sizeof(detalhe_hist), "equipamento %d | entrada", eq->id);
        registrar_historico(historico, "Manutencao", detalhe_hist, "");

        char desc_op[TAM_DESC_OP];
        char id_rel[TAM_ID_REL];
        snprintf(desc_op, sizeof(desc_op), "Equipamento %d enviado para manutencao", eq->id);
        snprintf(id_rel, sizeof(id_rel), "Equipamento %d", eq->id);
        empilhar_operacao(pilha_op, desc_op, "", id_rel);

        printf("Equipamento %d enviado para manutencao.\n", eq->id);
    } else if (eq->estado == ESTADO_MANUTENCAO) {
        eq->estado = ESTADO_DISPONIVEL;

        char detalhe_hist[TAM_DETALHE_MOV];
        snprintf(detalhe_hist, sizeof(detalhe_hist), "equipamento %d | saida", eq->id);
        registrar_historico(historico, "Manutencao", detalhe_hist, "");

        char desc_op[TAM_DESC_OP];
        char id_rel[TAM_ID_REL];
        snprintf(desc_op, sizeof(desc_op), "Equipamento %d retornou da manutencao", eq->id);
        snprintf(id_rel, sizeof(id_rel), "Equipamento %d", eq->id);
        empilhar_operacao(pilha_op, desc_op, "", id_rel);

        printf("Equipamento %d retornou da manutencao e esta disponivel.\n", eq->id);
    }
}

static void opcao_cancelar_solicitacao(FilaSolicitacoes *fila,
                                       PilhaOperacoes *pilha_op,
                                       ListaHistorico *historico) {
    int protocolo = 0;
    if (!ler_inteiro("Numero do protocolo a cancelar: ", &protocolo)) {
        return;
    }

    /* Fila vazia tambem cai aqui: cancelar_solicitacao retorna 0 */
    NoSolicitacao cancelada;
    if (!cancelar_solicitacao(fila, protocolo, &cancelada)) {
        printf("Erro: protocolo %d nao encontrado na fila.\n", protocolo);
        return;
    }

    char detalhe_hist[TAM_DETALHE_MOV];
    snprintf(detalhe_hist, sizeof(detalhe_hist),
             "protocolo %d | %s (%s)",
             cancelada.protocolo, cancelada.solicitante, cancelada.matricula);
    registrar_historico(historico, "Cancelamento", detalhe_hist, cancelada.data);

    char desc_op[TAM_DESC_OP];
    char id_rel[TAM_ID_REL];
    snprintf(desc_op, sizeof(desc_op),
             "Cancelamento do protocolo %d (%s)",
             cancelada.protocolo, cancelada.solicitante);
    snprintf(id_rel, sizeof(id_rel), "Protocolo %d", cancelada.protocolo);
    empilhar_operacao(pilha_op, desc_op, cancelada.data, id_rel);

    printf("Protocolo %d cancelado.\n", cancelada.protocolo);
}

static void opcao_mostrar_estado_sistema(const Equipamento *lista_eq,
                                         const FilaSolicitacoes *fila,
                                         const PilhaOperacoes *pilha_op,
                                         const ListaHistorico *historico) {
    printf("\n=== Equipamentos ===\n");
    listar_equipamentos(lista_eq);

    printf("\n=== Fila de solicitacoes ===\n");
    listar_solicitacoes(fila);

    printf("\n=== Operacoes recentes (topo primeiro) ===\n");
    listar_operacoes(pilha_op);

    printf("\n=== Historico de movimentacoes ===\n");
    listar_historico(historico);
}

static int opcao_salvar_dados(const Equipamento *lista_eq, const ListaHistorico *historico) {
    int salvos_eq = 0;
    int salvos_hist = 0;

    if (!salvar_equipamentos_bin(ARQUIVO_EQUIPAMENTOS, lista_eq, &salvos_eq)) {
        return 0;
    }

    if (!salvar_historico_bin(ARQUIVO_HISTORICO, historico, &salvos_hist)) {
        return 0;
    }

    printf("Dados salvos: %d equipamentos, %d movimentacoes.\n", salvos_eq, salvos_hist);
    return 1;
}

static void opcao_carregar_dados(Equipamento **lista_eq, ListaHistorico *historico) {
    int carregados_eq = 0;
    int carregados_hist = 0;

    if (!carregar_equipamentos_bin(ARQUIVO_EQUIPAMENTOS, lista_eq, &carregados_eq)) {
        return;
    }

    if (!carregar_historico_bin(ARQUIVO_HISTORICO, historico, &carregados_hist)) {
        printf("Aviso: equipamentos carregados, mas o historico nao foi carregado.\n");
        return;
    }

    printf("Dados carregados: %d equipamentos, %d movimentacoes.\n", carregados_eq, carregados_hist);
}

static void liberar_tudo(Equipamento **lista_eq,
                         FilaSolicitacoes *fila,
                         PilhaOperacoes *pilha_op,
                         ListaHistorico *historico) {
    liberar_equipamentos(lista_eq);
    liberar_fila(fila);
    liberar_pilha(pilha_op);
    liberar_historico(historico);
    printf("Memoria liberada. Encerrando o sistema.\n");
}

int main(void) {
    Equipamento *lista_eq = NULL;
    FilaSolicitacoes fila;
    PilhaOperacoes pilha_op;
    ListaHistorico historico;

    inicializar_fila(&fila);
    inicializar_pilha(&pilha_op);
    inicializar_historico(&historico);

    int opcao = -1;

    do {
        printf("\n=== CONTROLE DA LAMPISTERIA ===\n");
        printf("1. Cadastrar equipamento\n");
        printf("2. Listar equipamentos\n");
        printf("3. Buscar equipamento\n");
        printf("4. Registrar solicitacao\n");
        printf("5. Listar solicitacoes pendentes\n");
        printf("6. Atender proxima solicitacao\n");
        printf("7. Registrar devolucao\n");
        printf("8. Alterar manutencao\n");
        printf("9. Cancelar solicitacao\n");
        printf("10. Mostrar estado do sistema\n");
        printf("11. Salvar dados\n");
        printf("12. Carregar dados\n");
        printf("0. Sair\n");

        if (!ler_inteiro("Escolha uma opcao: ", &opcao)) {
            if (fim_entrada) {
                liberar_tudo(&lista_eq, &fila, &pilha_op, &historico);
                return 0;
            }
            opcao = -1;
            continue;
        }

        switch (opcao) {
            case 1:
                opcao_cadastrar_equipamento(&lista_eq, &pilha_op);
                break;
            case 2:
                printf("\n--- Equipamentos ---\n");
                listar_equipamentos(lista_eq);
                break;
            case 3:
                opcao_buscar_equipamento(lista_eq);
                break;
            case 4:
                opcao_registrar_solicitacao(&fila, lista_eq, &pilha_op);
                break;
            case 5:
                printf("\n--- Solicitacoes Pendentes ---\n");
                listar_solicitacoes(&fila);
                break;
            case 6:
                opcao_atender_solicitacao(&fila, lista_eq, &pilha_op, &historico);
                break;
            case 7:
                opcao_registrar_devolucao(lista_eq, &pilha_op, &historico);
                break;
            case 8:
                opcao_alterar_manutencao(lista_eq, &pilha_op, &historico);
                break;
            case 9:
                opcao_cancelar_solicitacao(&fila, &pilha_op, &historico);
                break;
            case 10:
                opcao_mostrar_estado_sistema(lista_eq, &fila, &pilha_op, &historico);
                break;
            case 11:
                opcao_salvar_dados(lista_eq, &historico);
                break;
            case 12:
                opcao_carregar_dados(&lista_eq, &historico);
                break;
            case 0: {
                char resp[16];
                ler_linha("Deseja salvar antes de sair? (s/n): ", resp, sizeof(resp));
                if (resp[0] == 's' || resp[0] == 'S') {
                    opcao_salvar_dados(lista_eq, &historico);
                }
                liberar_tudo(&lista_eq, &fila, &pilha_op, &historico);
                break;
            }
            default:
                printf("Opcao invalida.\n");
                break;
        }
    } while (opcao != 0);

    return 0;
}
