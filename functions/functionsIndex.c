/*
 * functionsIndex.c
 * Implementa as funcionalidades [5] a [9] do TP1:
 *   [5] Criar arquivo de índice primário
 *   [6] Busca com suporte de índice primário
 *   [7] Remoção lógica com pilha + atualização do índice
 *   [8] Inserção com reaproveitamento de espaço + atualização do índice
 *   [9] Atualização in-place + atualização do índice
 *
 * Todas as verificações de consistência (status dos arquivos) são feitas
 * em main.c antes de chamar estas funções, seguindo o mesmo padrão das
 * funcionalidades [2], [3] e [4].
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "index.h"
#include "functionsIndex.h"
#include "functions.h"
#include "header.h"
#include "registry.h"
#include "utils.h"

/* ================================================================
   ESTRUTURA AUXILIAR
   Par (nome do campo, valor) usado nas buscas e atualizações.
   ================================================================ */
typedef struct campo {
    char nome[100];  /**< nome do campo (ex: "codEstacao", "nomeLinha") */
    char valor[100]; /**< valor do campo como string (NULO vira "") */
} Campo;

/* ================================================================
   FUNÇÕES AUXILIARES INTERNAS
   Marcadas como static — visíveis apenas dentro deste arquivo.
   ================================================================ */

/* Comparador para o qsort: ordena entradas do índice por codEstacao crescente */
static int comparaEntradaIndice(const void *a, const void *b)
{
    const Index *ia = (const Index *)a;
    const Index *ib = (const Index *)b;
    return ia->codEstacao - ib->codEstacao;
}

/* Lê o cabeçalho completo do arquivo de dados para dentro de uma struct Header */
static void lerCabecalho(FILE *arq, Header *cab)
{
    fseek(arq, 0, SEEK_SET);
    fread(&cab->status,          sizeof(char), 1, arq);
    fread(&cab->topo,            sizeof(int),  1, arq);
    fread(&cab->proxRRN,         sizeof(int),  1, arq);
    fread(&cab->nroEstacoes,     sizeof(int),  1, arq);
    fread(&cab->nroParesEstacao, sizeof(int),  1, arq);
}

/* Escreve o cabeçalho completo de volta no arquivo de dados */
static void escreverCabecalho(FILE *arq, Header *cab)
{
    fseek(arq, 0, SEEK_SET);
    fwrite(&cab->status,          sizeof(char), 1, arq);
    fwrite(&cab->topo,            sizeof(int),  1, arq);
    fwrite(&cab->proxRRN,         sizeof(int),  1, arq);
    fwrite(&cab->nroEstacoes,     sizeof(int),  1, arq);
    fwrite(&cab->nroParesEstacao, sizeof(int),  1, arq);
}

/* Conta quantas entradas de dados existem no arquivo de índice.
   Desconta o byte do cabeçalho e divide pelo tamanho fixo de cada entrada. */
static int contarEntradasIndice(FILE *arqIndice)
{
    fseek(arqIndice, 0, SEEK_END);
    long tamanho = ftell(arqIndice);
    return (int)((tamanho - sizeof(char)) / INDEX_SIZE);
}

/*
 * Busca binária no arquivo de índice pelo codEstacao.
 * O índice está sempre ordenado por codEstacao crescente.
 * Retorna o RRN correspondente se encontrar, ou -1 se não encontrar.
 */
static int buscaBinariaIndice(FILE *arqIndice, int codEstacao)
{
    int total = contarEntradasIndice(arqIndice);
    int esq = 0, dir = total - 1;

    while (esq <= dir) {
        int meio = (esq + dir) / 2;

        /* pula o byte do cabeçalho e salta direto à entrada do meio */
        fseek(arqIndice, sizeof(char) + (long)meio * INDEX_SIZE, SEEK_SET);

        Index entrada;
        fread(&entrada.codEstacao, sizeof(int), 1, arqIndice);
        fread(&entrada.RRN,        sizeof(int), 1, arqIndice);

        if (entrada.codEstacao == codEstacao)
            return entrada.RRN;
        else if (entrada.codEstacao < codEstacao)
            esq = meio + 1;
        else
            dir = meio - 1;
    }

    return -1; /* não encontrou */
}

/*
 * Remove a entrada com o codEstacao informado do arquivo de índice.
 * Lê todas as entradas para a memória, reescreve pulando a removida,
 * e trunca o arquivo no novo tamanho correto para não deixar lixo no final.
 */
static void removerDoIndice(FILE *arqIndice, int codEstacao)
{
    int total = contarEntradasIndice(arqIndice);
    if (total == 0) return;

    Index *entradas = malloc(total * sizeof(Index));

    /* carrega todas as entradas do índice na memória */
    fseek(arqIndice, sizeof(char), SEEK_SET);
    for (int i = 0; i < total; i++) {
        fread(&entradas[i].codEstacao, sizeof(int), 1, arqIndice);
        fread(&entradas[i].RRN,        sizeof(int), 1, arqIndice);
    }

    /* reescreve o índice pulando a entrada a ser removida */
    fseek(arqIndice, sizeof(char), SEEK_SET);
    int novoTotal = 0;
    for (int i = 0; i < total; i++) {
        if (entradas[i].codEstacao != codEstacao) {
            fwrite(&entradas[i].codEstacao, sizeof(int), 1, arqIndice);
            fwrite(&entradas[i].RRN,        sizeof(int), 1, arqIndice);
            novoTotal++;
        }
    }

    /* trunca o arquivo para eliminar o registro duplicado que sobrou no final */
    fflush(arqIndice);
    ftruncate(fileno(arqIndice), sizeof(char) + (long)novoTotal * INDEX_SIZE);

    free(entradas);
}

/*
 * Insere uma nova entrada no arquivo de índice mantendo a ordem crescente por codEstacao.
 * Lê tudo para a memória, encontra a posição de inserção, abre espaço deslocando as
 * entradas maiores para a direita, e reescreve o arquivo completo.
 */
static void inserirNoIndice(FILE *arqIndice, int codEstacao, int rrn)
{
    int total = contarEntradasIndice(arqIndice);
    Index *entradas = malloc((total + 1) * sizeof(Index));

    /* carrega as entradas existentes */
    fseek(arqIndice, sizeof(char), SEEK_SET);
    for (int i = 0; i < total; i++) {
        fread(&entradas[i].codEstacao, sizeof(int), 1, arqIndice);
        fread(&entradas[i].RRN,        sizeof(int), 1, arqIndice);
    }

    /* encontra onde inserir para manter a ordem crescente */
    int posInsercao = total;
    for (int i = 0; i < total; i++) {
        if (entradas[i].codEstacao > codEstacao) {
            posInsercao = i;
            break;
        }
    }

    /* abre espaço deslocando as entradas maiores uma posição para a direita */
    for (int i = total; i > posInsercao; i--)
        entradas[i] = entradas[i - 1];

    /* preenche a nova entrada na posição correta */
    entradas[posInsercao].codEstacao = codEstacao;
    entradas[posInsercao].RRN        = rrn;

    /* reescreve o arquivo de índice inteiro com a nova entrada incluída */
    fseek(arqIndice, sizeof(char), SEEK_SET);
    for (int i = 0; i <= total; i++) {
        fwrite(&entradas[i].codEstacao, sizeof(int), 1, arqIndice);
        fwrite(&entradas[i].RRN,        sizeof(int), 1, arqIndice);
    }

    free(entradas);
}

/*
 * Verifica se um registro satisfaz todos os critérios de busca informados.
 * Campos inteiros com valor "" (vindo de NULO) são comparados como -1.
 * Retorna 1 se satisfaz todos os critérios, 0 se falhar em qualquer um.
 */
static int registroCorresponde(Registry *reg, Campo *campos, int m)
{
    for (int k = 0; k < m; k++) {
        if (strcmp(campos[k].nome, "codEstacao") == 0) {
            int val = (strcmp(campos[k].valor, "") == 0) ? -1 : atoi(campos[k].valor);
            if (reg->codEstacao != val) return 0;
        }
        else if (strcmp(campos[k].nome, "codLinha") == 0) {
            int val = (strcmp(campos[k].valor, "") == 0) ? -1 : atoi(campos[k].valor);
            if (reg->codLinha != val) return 0;
        }
        else if (strcmp(campos[k].nome, "codProxEstacao") == 0) {
            int val = (strcmp(campos[k].valor, "") == 0) ? -1 : atoi(campos[k].valor);
            if (reg->codProxEstacao != val) return 0;
        }
        else if (strcmp(campos[k].nome, "distProxEstacao") == 0) {
            int val = (strcmp(campos[k].valor, "") == 0) ? -1 : atoi(campos[k].valor);
            if (reg->distProxEstacao != val) return 0;
        }
        else if (strcmp(campos[k].nome, "codLinhaIntegra") == 0) {
            int val = (strcmp(campos[k].valor, "") == 0) ? -1 : atoi(campos[k].valor);
            if (reg->codLinhaIntegra != val) return 0;
        }
        else if (strcmp(campos[k].nome, "codEstIntegra") == 0) {
            int val = (strcmp(campos[k].valor, "") == 0) ? -1 : atoi(campos[k].valor);
            if (reg->codEstIntegra != val) return 0;
        }
        else if (strcmp(campos[k].nome, "nomeEstacao") == 0) {
            if (reg->tamNomeEstacao == 0) {
                if (strcmp(campos[k].valor, "") != 0) return 0;
            } else {
                if (strcmp(reg->nomeEstacao, campos[k].valor) != 0) return 0;
            }
        }
        else if (strcmp(campos[k].nome, "nomeLinha") == 0) {
            if (reg->tamNomeLinha == 0) {
                if (strcmp(campos[k].valor, "") != 0) return 0;
            } else {
                if (strcmp(reg->nomeLinha, campos[k].valor) != 0) return 0;
            }
        }
    }
    return 1;
}

/*
 * Verifica se o campo "codEstacao" está entre os critérios de busca.
 * Se estiver, retorna o valor inteiro buscado (NULO vira -1).
 * Se não estiver, retorna -2 como sentinela de "não há codEstacao na busca".
 */
static int obterCodEstacaoBusca(Campo *campos, int m)
{
    for (int k = 0; k < m; k++) {
        if (strcmp(campos[k].nome, "codEstacao") == 0)
            return (strcmp(campos[k].valor, "") == 0) ? -1 : atoi(campos[k].valor);
    }
    return -2;
}

/*
 * Lê os 8 campos de um novo registro direto do stdin na ordem especificada:
 * codEstacao, nomeEstacao, codLinha, nomeLinha, codProxEstacao,
 * distProxEstacao, codLinhaIntegra, codEstIntegra.
 * Usa ScanQuoteString para tratar strings entre aspas e valores NULO.
 * Campos inteiros NULO viram -1; campos string NULO ficam como NULL.
 */
static void lerRegistroStdin(Registry *reg)
{
    char buf[500];

    ScanQuoteString(buf);
    reg->codEstacao = (strcmp(buf, "") == 0) ? -1 : atoi(buf);

    ScanQuoteString(buf);
    if (strcmp(buf, "") == 0) {
        reg->tamNomeEstacao = 0;
        reg->nomeEstacao    = NULL;
    } else {
        reg->tamNomeEstacao = strlen(buf);
        reg->nomeEstacao    = strdup(buf);
    }

    ScanQuoteString(buf);
    reg->codLinha = (strcmp(buf, "") == 0) ? -1 : atoi(buf);

    ScanQuoteString(buf);
    if (strcmp(buf, "") == 0) {
        reg->tamNomeLinha = 0;
        reg->nomeLinha    = NULL;
    } else {
        reg->tamNomeLinha = strlen(buf);
        reg->nomeLinha    = strdup(buf);
    }

    ScanQuoteString(buf);
    reg->codProxEstacao = (strcmp(buf, "") == 0) ? -1 : atoi(buf);

    ScanQuoteString(buf);
    reg->distProxEstacao = (strcmp(buf, "") == 0) ? -1 : atoi(buf);

    ScanQuoteString(buf);
    reg->codLinhaIntegra = (strcmp(buf, "") == 0) ? -1 : atoi(buf);

    ScanQuoteString(buf);
    reg->codEstIntegra = (strcmp(buf, "") == 0) ? -1 : atoi(buf);

    reg->removido = IS_NOT_REMOVED;
    reg->proximo  = -1;
}

/*
 * Aplica os pares (campo, novo valor) sobre o registro em memória.
 * Para campos string, libera o valor antigo antes de atribuir o novo.
 * NULO vira -1 para inteiros e NULL para strings.
 */
static void aplicarAtualizacoes(Registry *reg, Campo *camposAtu, int p)
{
    for (int k = 0; k < p; k++) {
        if (strcmp(camposAtu[k].nome, "codEstacao") == 0) {
            reg->codEstacao = (strcmp(camposAtu[k].valor, "") == 0) ? -1 : atoi(camposAtu[k].valor);
        }
        else if (strcmp(camposAtu[k].nome, "codLinha") == 0) {
            reg->codLinha = (strcmp(camposAtu[k].valor, "") == 0) ? -1 : atoi(camposAtu[k].valor);
        }
        else if (strcmp(camposAtu[k].nome, "codProxEstacao") == 0) {
            reg->codProxEstacao = (strcmp(camposAtu[k].valor, "") == 0) ? -1 : atoi(camposAtu[k].valor);
        }
        else if (strcmp(camposAtu[k].nome, "distProxEstacao") == 0) {
            reg->distProxEstacao = (strcmp(camposAtu[k].valor, "") == 0) ? -1 : atoi(camposAtu[k].valor);
        }
        else if (strcmp(camposAtu[k].nome, "codLinhaIntegra") == 0) {
            reg->codLinhaIntegra = (strcmp(camposAtu[k].valor, "") == 0) ? -1 : atoi(camposAtu[k].valor);
        }
        else if (strcmp(camposAtu[k].nome, "codEstIntegra") == 0) {
            reg->codEstIntegra = (strcmp(camposAtu[k].valor, "") == 0) ? -1 : atoi(camposAtu[k].valor);
        }
        else if (strcmp(camposAtu[k].nome, "nomeEstacao") == 0) {
            if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
            if (strcmp(camposAtu[k].valor, "") == 0) {
                reg->tamNomeEstacao = 0;
                reg->nomeEstacao    = NULL;
            } else {
                reg->tamNomeEstacao = strlen(camposAtu[k].valor);
                reg->nomeEstacao    = strdup(camposAtu[k].valor);
            }
        }
        else if (strcmp(camposAtu[k].nome, "nomeLinha") == 0) {
            if (reg->tamNomeLinha > 0) free(reg->nomeLinha);
            if (strcmp(camposAtu[k].valor, "") == 0) {
                reg->tamNomeLinha = 0;
                reg->nomeLinha    = NULL;
            } else {
                reg->tamNomeLinha = strlen(camposAtu[k].valor);
                reg->nomeLinha    = strdup(camposAtu[k].valor);
            }
        }
    }
}

/* ================================================================
   [5] CRIAR ARQUIVO DE ÍNDICE PRIMÁRIO
   Percorre o arquivo de dados, coleta todos os pares (codEstacao, RRN)
   de registros não removidos, ordena por codEstacao e escreve no índice.
   O status dos arquivos já foi verificado em main.c antes desta chamada.
   ================================================================ */
int createPrimaryIndexArchive(FILE *registryBinaryFile, FILE *primaryIndexArchive)
{
    if (registryBinaryFile == NULL || primaryIndexArchive == NULL)
        return FUNCTION_FAILURE;

    /* marca o arquivo de índice como inconsistente durante a construção */
    fseek(primaryIndexArchive, 0, SEEK_SET);
    char statusIndice = INDEX_INCONSISTENT;
    fwrite(&statusIndice, sizeof(char), 1, primaryIndexArchive);

    /* aloca espaço para guardar os pares (codEstacao, RRN) temporariamente */
    int capacidade = 10000;
    Index *entradas = malloc(capacidade * sizeof(Index));
    int total = 0;

    /* começa a leitura logo após o cabeçalho do arquivo de dados */
    fseek(registryBinaryFile, HEADER_SIZE, SEEK_SET);
    Registry reg;
    int rrn = 0;

    /* percorre todos os registros do arquivo de dados */
    while (binaryToRegistry(&reg, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS) {
        /* só inclui no índice registros que não foram removidos logicamente */
        if (reg.removido == IS_NOT_REMOVED) {
            entradas[total].codEstacao = reg.codEstacao;
            entradas[total].RRN        = rrn;
            total++;
        }
        freeRegistry(&reg);
        rrn++;
    }

    /* ordena as entradas coletadas por codEstacao em ordem crescente */
    qsort(entradas, total, sizeof(Index), comparaEntradaIndice);

    /* escreve as entradas ordenadas no arquivo de índice campo a campo */
    for (int i = 0; i < total; i++) {
        fwrite(&entradas[i].codEstacao, sizeof(int), 1, primaryIndexArchive);
        fwrite(&entradas[i].RRN,        sizeof(int), 1, primaryIndexArchive);
    }

    /* terminou sem erros — marca o arquivo de índice como consistente */
    fseek(primaryIndexArchive, 0, SEEK_SET);
    statusIndice = INDEX_CONSISTENT;
    fwrite(&statusIndice, sizeof(char), 1, primaryIndexArchive);

    free(entradas);
    return FUNCTION_SUCESS;
}

/* ================================================================
   [6] BUSCA COM SUPORTE DE ÍNDICE PRIMÁRIO
   Se o campo 'codEstacao' estiver na busca, usa busca binária no índice
   para ir direto ao RRN. Caso contrário, faz varredura sequencial.
   O status dos arquivos já foi verificado em main.c antes desta chamada.
   ================================================================ */
int restoreIndexArchive(FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches)
{
    if (registryBinaryFile == NULL || primaryIndexArchive == NULL)
        return FUNCTION_FAILURE;

    for (int i = 0; i < numberOfSearches; i++) {
        int m;
        scanf("%d", &m);

        Campo campos[m];
        for (int j = 0; j < m; j++) {
            scanf("%s", campos[j].nome);
            ScanQuoteString(campos[j].valor);
        }

        int achouPeloMenosUm = 0;

        /* verifica se o critério de busca inclui o campo codEstacao */
        int codBuscado = obterCodEstacaoBusca(campos, m);

        if (codBuscado != -2) {
            /* BUSCA INDEXADA: usa o índice para encontrar o RRN diretamente */
            int rrn = buscaBinariaIndice(primaryIndexArchive, codBuscado);
            if (rrn >= 0) {
                fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                Registry reg;
                if (binaryToRegistry(&reg, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS) {
                    /* confirma que o registro não foi removido e satisfaz os outros filtros */
                    if (reg.removido == IS_NOT_REMOVED && registroCorresponde(&reg, campos, m)) {
                        printRegistry(&reg);
                        achouPeloMenosUm = 1;
                    }
                    freeRegistry(&reg);
                }
            }
        } else {
            /* BUSCA SEQUENCIAL: percorre todos os registros do arquivo de dados */
            fseek(registryBinaryFile, HEADER_SIZE, SEEK_SET);
            Registry reg;
            while (binaryToRegistry(&reg, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS) {
                if (reg.removido == IS_NOT_REMOVED && registroCorresponde(&reg, campos, m)) {
                    printRegistry(&reg);
                    achouPeloMenosUm++;
                }
                freeRegistry(&reg);
            }
        }

        if (!achouPeloMenosUm)
            printf("Registro inexistente.\n");

        /* separa as saídas de buscas consecutivas com uma linha em branco */
        if (i < numberOfSearches - 1)
            printf("\n");
    }

    return FUNCTION_SUCESS;
}

/* ================================================================
   [7] REMOÇÃO LÓGICA COM PILHA + ATUALIZAÇÃO DO ÍNDICE
   Encontra os registros que satisfazem o critério (indexada ou sequencial),
   marca como removidos usando a pilha (topo/proximo no cabeçalho),
   e remove as entradas correspondentes do arquivo de índice.
   O status dos arquivos já foi verificado em main.c antes desta chamada.
   ================================================================ */
int removeIndexArchive(FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches)
{
    if (registryBinaryFile == NULL || primaryIndexArchive == NULL)
        return FUNCTION_FAILURE;

    /* marca ambos os arquivos como inconsistentes enquanto fazemos alterações —
       protege contra queda de energia ou interrupção durante a operação */
    char inconsistente = STATUS_INCONSISTENT;
    fseek(registryBinaryFile, 0, SEEK_SET);
    fwrite(&inconsistente, sizeof(char), 1, registryBinaryFile);

    char inconsistenteIndice = INDEX_INCONSISTENT;
    fseek(primaryIndexArchive, 0, SEEK_SET);
    fwrite(&inconsistenteIndice, sizeof(char), 1, primaryIndexArchive);

    /* lê o cabeçalho do arquivo de dados para ter acesso ao topo da pilha */
    Header cab;
    lerCabecalho(registryBinaryFile, &cab);

    for (int i = 0; i < numberOfSearches; i++) {
        int m;
        scanf("%d", &m);

        Campo campos[m];
        for (int j = 0; j < m; j++) {
            scanf("%s", campos[j].nome);
            ScanQuoteString(campos[j].valor);
        }

        int codBuscado = obterCodEstacaoBusca(campos, m);

        if (codBuscado != -2) {
            /* BUSCA INDEXADA */
            int rrn = buscaBinariaIndice(primaryIndexArchive, codBuscado);
            if (rrn >= 0) {
                fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                Registry reg;
                if (binaryToRegistry(&reg, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS) {
                    if (reg.removido == IS_NOT_REMOVED && registroCorresponde(&reg, campos, m)) {
                        int cod = reg.codEstacao;
                        freeRegistry(&reg);

                        /* sobrescreve APENAS o campo removido e o campo proximo no início do
                           registro. Os demais bytes permanecem inalterados, conforme a especificação.
                           proximo aponta para o antigo topo — empilha este registro na pilha. */
                        fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                        char marcaRemovido = IS_REMOVED;
                        fwrite(&marcaRemovido, sizeof(char), 1, registryBinaryFile);
                        fwrite(&cab.topo,      sizeof(int),  1, registryBinaryFile);
                        cab.topo = rrn; /* este RRN passa a ser o novo topo da pilha */

                        removerDoIndice(primaryIndexArchive, cod);
                    } else {
                        freeRegistry(&reg);
                    }
                }
            }
        } else {
            /* BUSCA SEQUENCIAL: posiciona uma vez no início e avança registro a registro.
               fseek só é feito quando escrevemos no arquivo e movemos o cursor. */
            fseek(registryBinaryFile, HEADER_SIZE, SEEK_SET);
            int rrn = 0;
            while (rrn < cab.proxRRN) {
                Registry reg;
                if (binaryToRegistry(&reg, registryBinaryFile) != BINARY_TO_REGISTRY_SUCESS)
                    break;

                if (reg.removido == IS_NOT_REMOVED && registroCorresponde(&reg, campos, m)) {
                    int cod = reg.codEstacao;
                    freeRegistry(&reg);

                    /* volta ao início deste registro para sobrescrever removido e proximo.
                       Os demais bytes permanecem inalterados, conforme a especificação. */
                    fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                    char marcaRemovido = IS_REMOVED;
                    fwrite(&marcaRemovido, sizeof(char), 1, registryBinaryFile);
                    fwrite(&cab.topo,      sizeof(int),  1, registryBinaryFile);
                    cab.topo = rrn;

                    /* após escrever 5 bytes, o cursor está no meio do registro —
                       avança os bytes restantes para chegar ao próximo registro */
                    fseek(registryBinaryFile, REGISTRY_SIZE - sizeof(char) - sizeof(int), SEEK_CUR);

                    removerDoIndice(primaryIndexArchive, cod);
                } else {
                    freeRegistry(&reg);
                    /* binaryToRegistry já posicionou o cursor no próximo registro */
                }
                rrn++;
            }
        }
    }

    /* restaura o cabeçalho com o topo atualizado e marca o arquivo como consistente */
    cab.status = STATUS_CONSISTENT;
    escreverCabecalho(registryBinaryFile, &cab);

    /* marca o arquivo de índice como consistente */
    fseek(primaryIndexArchive, 0, SEEK_SET);
    char consistenteIndice = INDEX_CONSISTENT;
    fwrite(&consistenteIndice, sizeof(char), 1, primaryIndexArchive);

    return FUNCTION_SUCESS;
}

/* ================================================================
   [8] INSERÇÃO COM REAPROVEITAMENTO DE ESPAÇO (PILHA) + ÍNDICE
   Para cada novo registro: se a pilha não estiver vazia, reutiliza o
   espaço do topo (lê o proximo para desempilhar); senão, acrescenta ao
   final do arquivo incrementando proxRRN. Insere no índice em ordem.
   O status dos arquivos já foi verificado em main.c antes desta chamada.
   ================================================================ */
int insertNewIndexArchive(FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfRegistries)
{
    if (registryBinaryFile == NULL || primaryIndexArchive == NULL)
        return FUNCTION_FAILURE;

    /* marca ambos os arquivos como inconsistentes durante as inserções */
    char inconsistente = STATUS_INCONSISTENT;
    fseek(registryBinaryFile, 0, SEEK_SET);
    fwrite(&inconsistente, sizeof(char), 1, registryBinaryFile);

    char inconsistenteIndice = INDEX_INCONSISTENT;
    fseek(primaryIndexArchive, 0, SEEK_SET);
    fwrite(&inconsistenteIndice, sizeof(char), 1, primaryIndexArchive);

    /* lê o cabeçalho para saber o estado atual da pilha e do proxRRN */
    Header cab;
    lerCabecalho(registryBinaryFile, &cab);

    for (int i = 0; i < numberOfRegistries; i++) {
        Registry reg;
        lerRegistroStdin(&reg); /* lê os 8 campos do novo registro pelo stdin */

        int rrnNovo;

        if (cab.topo != -1) {
            /* a pilha tem espaço livre — reutiliza o slot do topo */
            rrnNovo = cab.topo;

            /* lê o campo proximo do registro removido para desempilhar.
               proximo fica logo após o byte removido, no offset +1 do início do registro. */
            fseek(registryBinaryFile,
                  HEADER_SIZE + (long)rrnNovo * REGISTRY_SIZE + sizeof(char),
                  SEEK_SET);
            fread(&cab.topo, sizeof(int), 1, registryBinaryFile);

        } else {
            /* pilha vazia — acrescenta ao final do arquivo de dados */
            rrnNovo = cab.proxRRN;
            cab.proxRRN++;
        }

        /* guarda o codEstacao antes de chamar registryToBinary, pois ela libera os ponteiros */
        int codEstacaoNovo = reg.codEstacao;

        /* escreve o novo registro na posição calculada do arquivo de dados */
        fseek(registryBinaryFile, HEADER_SIZE + (long)rrnNovo * REGISTRY_SIZE, SEEK_SET);
        registryToBinary(&reg, registryBinaryFile);

        /* insere no arquivo de índice mantendo a ordem crescente por codEstacao */
        inserirNoIndice(primaryIndexArchive, codEstacaoNovo, rrnNovo);
    }

    /* restaura o cabeçalho com proxRRN e topo atualizados */
    cab.status = STATUS_CONSISTENT;
    escreverCabecalho(registryBinaryFile, &cab);

    /* marca o arquivo de índice como consistente */
    fseek(primaryIndexArchive, 0, SEEK_SET);
    char consistenteIndice = INDEX_CONSISTENT;
    fwrite(&consistenteIndice, sizeof(char), 1, primaryIndexArchive);

    return FUNCTION_SUCESS;
}

/* ================================================================
   [9] ATUALIZAÇÃO IN-PLACE + ÍNDICE
   Para cada operação: lê m critérios de busca e p campos a atualizar.
   Encontra os registros que satisfazem a busca (indexada ou sequencial),
   aplica as alterações diretamente no slot de 80 bytes (in-place),
   e atualiza o índice se o codEstacao foi modificado.
   O status dos arquivos já foi verificado em main.c antes desta chamada.
   ================================================================ */
int updateIndexArchive(FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfUpdates)
{
    if (registryBinaryFile == NULL || primaryIndexArchive == NULL)
        return FUNCTION_FAILURE;

    /* marca ambos os arquivos como inconsistentes durante as atualizações */
    char inconsistente = STATUS_INCONSISTENT;
    fseek(registryBinaryFile, 0, SEEK_SET);
    fwrite(&inconsistente, sizeof(char), 1, registryBinaryFile);

    char inconsistenteIndice = INDEX_INCONSISTENT;
    fseek(primaryIndexArchive, 0, SEEK_SET);
    fwrite(&inconsistenteIndice, sizeof(char), 1, primaryIndexArchive);

    /* lê o cabeçalho para saber quantos registros existem (proxRRN) */
    Header cab;
    lerCabecalho(registryBinaryFile, &cab);

    for (int i = 0; i < numberOfUpdates; i++) {
        /* m = quantidade de pares de busca (campo/valor) */
        int m;
        scanf("%d", &m);
        Campo camposBusca[m];
        for (int j = 0; j < m; j++) {
            scanf("%s", camposBusca[j].nome);
            ScanQuoteString(camposBusca[j].valor);
        }

        /* p = quantidade de pares de atualização (campo/novo valor) */
        int p;
        scanf("%d", &p);
        Campo camposAtu[p];
        for (int j = 0; j < p; j++) {
            scanf("%s", camposAtu[j].nome);
            ScanQuoteString(camposAtu[j].valor);
        }

        int codBuscado = obterCodEstacaoBusca(camposBusca, m);

        if (codBuscado != -2) {
            /* BUSCA INDEXADA */
            int rrn = buscaBinariaIndice(primaryIndexArchive, codBuscado);
            if (rrn >= 0) {
                fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                Registry reg;
                if (binaryToRegistry(&reg, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS) {
                    if (reg.removido == IS_NOT_REMOVED && registroCorresponde(&reg, camposBusca, m)) {
                        int codAntigo = reg.codEstacao;

                        /* aplica as atualizações sobre o registro em memória */
                        aplicarAtualizacoes(&reg, camposAtu, p);

                        int codNovo = reg.codEstacao;

                        /* reescreve o registro inteiro na mesma posição (in-place) */
                        fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                        registryToBinary(&reg, registryBinaryFile);

                        /* se o codEstacao mudou, atualiza a entrada no arquivo de índice */
                        if (codNovo != codAntigo) {
                            removerDoIndice(primaryIndexArchive, codAntigo);
                            inserirNoIndice(primaryIndexArchive, codNovo, rrn);
                        }
                    } else {
                        freeRegistry(&reg);
                    }
                }
            }
        } else {
            /* BUSCA SEQUENCIAL: posiciona uma vez no início e avança registro a registro.
               fseek só é feito quando escrevemos no arquivo e movemos o cursor. */
            fseek(registryBinaryFile, HEADER_SIZE, SEEK_SET);
            int rrn = 0;
            while (rrn < cab.proxRRN) {
                Registry reg;
                if (binaryToRegistry(&reg, registryBinaryFile) != BINARY_TO_REGISTRY_SUCESS)
                    break;

                if (reg.removido == IS_NOT_REMOVED && registroCorresponde(&reg, camposBusca, m)) {
                    int codAntigo = reg.codEstacao;

                    aplicarAtualizacoes(&reg, camposAtu, p);

                    int codNovo = reg.codEstacao;

                    /* volta ao início deste registro e reescreve os 80 bytes completos.
                       registryToBinary escreve exatamente REGISTRY_SIZE bytes, então o
                       cursor fica automaticamente no início do próximo registro. */
                    fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                    registryToBinary(&reg, registryBinaryFile);

                    if (codNovo != codAntigo) {
                        removerDoIndice(primaryIndexArchive, codAntigo);
                        inserirNoIndice(primaryIndexArchive, codNovo, rrn);
                    }
                } else {
                    freeRegistry(&reg);
                    /* binaryToRegistry já posicionou o cursor no próximo registro */
                }
                rrn++;
            }
        }
    }

    /* restaura o cabeçalho do arquivo de dados como consistente */
    cab.status = STATUS_CONSISTENT;
    escreverCabecalho(registryBinaryFile, &cab);

    /* marca o arquivo de índice como consistente */
    fseek(primaryIndexArchive, 0, SEEK_SET);
    char consistenteIndice = INDEX_CONSISTENT;
    fwrite(&consistenteIndice, sizeof(char), 1, primaryIndexArchive);

    return FUNCTION_SUCESS;
}
