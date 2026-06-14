#include <stdio.h>

#include "registry.h"
#include "index.h"
#include "header.h"
#include "utils.h"

//percorre todos os registros validos e conta:
//  nEst   -> quantos nomes de estacao distintos existem
//  nPares -> quantos pares (codEstacao, codProxEstacao) distintos existem
//isso e usado para calcular o quanto nroEstacoes e nroParesEstacao mudam
//apos uma insercao, remocao ou atualizacao (calculo via delta antes/depois)
void contarUnicosValidos(FILE *arq, int *nEst, int *nPares)
{
    char **uniqueStations = malloc(5000 * sizeof(char *));
    int numUniqueStations = 0;
    Pair *uniquePairs = malloc(5000 * sizeof(Pair));
    int numUniquePairs = 0;

    //posiciona logo apos o cabecalho e le registro por registro
    fseek(arq, HEADER_SIZE, SEEK_SET);
    Registry reg;
    while (binaryToRegistry(&reg, arq) == BINARY_TO_REGISTRY_SUCESS) {
        if (reg.removido == IS_NOT_REMOVED) {
            //adiciona o nome da estacao na lista de unicos se ainda nao estiver
            if (reg.tamNomeEstacao > 0)
                addUniqueStation(uniqueStations, &numUniqueStations, reg.nomeEstacao);
            //adiciona o par (cod, proxCod) na lista de pares unicos
            addUniquePair(uniquePairs, &numUniquePairs, reg.codEstacao, reg.codProxEstacao);
        }
        freeRegistry(&reg);
    }

    *nEst   = numUniqueStations;
    *nPares = numUniquePairs;

    //libera a memoria alocada para as listas de unicos
    for (int i = 0; i < numUniqueStations; i++)
        free(uniqueStations[i]);
    free(uniqueStations);
    free(uniquePairs);
}

//calcula quantas entradas de dados ha no arquivo de indice
//tamanho total menos 1 byte de cabecalho dividido por 8 bytes por entrada
int contarEntradasIndice(FILE *arqIndice)
{
    fseek(arqIndice, 0, SEEK_END);
    long tamanho = ftell(arqIndice);
    return (int)((tamanho - sizeof(char)) / INDEX_SIZE);
}

//remove do arquivo de indice a entrada com o codEstacao informado
//estrategia: carrega tudo na memoria, reescreve pulando a entrada removida,
//depois trunca o arquivo para o novo tamanho (sem isso sobra lixo no final)
void removerDoIndice(FILE *arqIndice, int codEstacao)
{
    int total = contarEntradasIndice(arqIndice);
    if (total == 0) return;

    Index *entradas = malloc(total * sizeof(Index));

    //carrega todas as entradas do indice para a memoria
    fseek(arqIndice, sizeof(char), SEEK_SET);
    for (int i = 0; i < total; i++) {
        fread(&entradas[i].codEstacao, sizeof(int), 1, arqIndice);
        fread(&entradas[i].RRN,        sizeof(int), 1, arqIndice);
    }

    //reescreve a partir do byte 1 pulando a entrada cujo codEstacao bate
    fseek(arqIndice, sizeof(char), SEEK_SET);
    int novoTotal = 0;
    for (int i = 0; i < total; i++) {
        if (entradas[i].codEstacao != codEstacao) {
            fwrite(&entradas[i].codEstacao, sizeof(int), 1, arqIndice);
            fwrite(&entradas[i].RRN,        sizeof(int), 1, arqIndice);
            novoTotal++;
        }
    }

    //fflush descarrega o buffer do stdio antes do truncamento de nivel de sistema operacional
    //ftruncate encurta o arquivo para o tamanho exato, eliminando o registro sobrando no final
    fflush(arqIndice);
    ftruncate(fileno(arqIndice), sizeof(char) + (long)novoTotal * INDEX_SIZE);

    free(entradas);
}

//insere uma nova entrada no arquivo de indice mantendo a ordem crescente por codEstacao
//estrategia: carrega tudo na memoria, acha onde inserir, empurra os maiores uma posicao
//para a direita abrindo espaco, depois reescreve o arquivo inteiro com a nova entrada
void inserirNoIndice(FILE *arqIndice, int codEstacao, int rrn)
{
    int total = contarEntradasIndice(arqIndice);
    //aloca espaco para total + 1 porque vai acrescentar uma entrada nova
    Index *entradas = malloc((total + 1) * sizeof(Index));

    //carrega as entradas existentes
    fseek(arqIndice, sizeof(char), SEEK_SET);
    for (int i = 0; i < total; i++) {
        fread(&entradas[i].codEstacao, sizeof(int), 1, arqIndice);
        fread(&entradas[i].RRN,        sizeof(int), 1, arqIndice);
    }

    //encontra a primeira posicao cujo codEstacao e maior que o novo
    //se nao encontrar a insercao vai para o final (posInsercao = total)
    int posInsercao = total;
    for (int i = 0; i < total; i++) {
        if (entradas[i].codEstacao > codEstacao) {
            posInsercao = i;
            break;
        }
    }

    //desloca as entradas maiores uma posicao para a direita abrindo o slot
    for (int i = total; i > posInsercao; i--)
        entradas[i] = entradas[i - 1];

    //preenche o slot aberto com a nova entrada
    entradas[posInsercao].codEstacao = codEstacao;
    entradas[posInsercao].RRN        = rrn;

    //reescreve o arquivo de indice completo com a nova entrada ja incluida
    fseek(arqIndice, sizeof(char), SEEK_SET);
    for (int i = 0; i <= total; i++) {
        fwrite(&entradas[i].codEstacao, sizeof(int), 1, arqIndice);
        fwrite(&entradas[i].RRN,        sizeof(int), 1, arqIndice);
    }

    free(entradas);
}

//aplica os p pares (campo, novo valor) sobre o registro que esta em memoria
//para campos string: libera a string antiga antes de alocar a nova
//valor vazio "" vira -1 para inteiros e NULL para strings (campo nulo)
void aplicarAtualizacoes(Registry *reg, Field *camposAtu, int p)
{
    for (int k = 0; k < p; k++) {
        if (strcmp(camposAtu[k].nameOfTheField, "codEstacao") == 0) {
            reg->codEstacao = (strcmp(camposAtu[k].valueOfTheField, "") == 0) ? -1 : atoi(camposAtu[k].valueOfTheField);
        }
        else if (strcmp(camposAtu[k].nameOfTheField, "codLinha") == 0) {
            reg->codLinha = (strcmp(camposAtu[k].valueOfTheField, "") == 0) ? -1 : atoi(camposAtu[k].valueOfTheField);
        }
        else if (strcmp(camposAtu[k].nameOfTheField, "codProxEstacao") == 0) {
            reg->codProxEstacao = (strcmp(camposAtu[k].valueOfTheField, "") == 0) ? -1 : atoi(camposAtu[k].valueOfTheField);
        }
        else if (strcmp(camposAtu[k].nameOfTheField, "distProxEstacao") == 0) {
            reg->distProxEstacao = (strcmp(camposAtu[k].valueOfTheField, "") == 0) ? -1 : atoi(camposAtu[k].valueOfTheField);
        }
        else if (strcmp(camposAtu[k].nameOfTheField, "codLinhaIntegra") == 0) {
            reg->codLinhaIntegra = (strcmp(camposAtu[k].valueOfTheField, "") == 0) ? -1 : atoi(camposAtu[k].valueOfTheField);
        }
        else if (strcmp(camposAtu[k].nameOfTheField, "codEstIntegra") == 0) {
            reg->codEstIntegra = (strcmp(camposAtu[k].valueOfTheField, "") == 0) ? -1 : atoi(camposAtu[k].valueOfTheField);
        }
        else if (strcmp(camposAtu[k].nameOfTheField, "nomeEstacao") == 0) {
            if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao); //libera a string antiga
            if (strcmp(camposAtu[k].valueOfTheField, "") == 0) {
                reg->tamNomeEstacao = 0;
                reg->nomeEstacao    = NULL;
            } else {
                reg->tamNomeEstacao = strlen(camposAtu[k].valueOfTheField);
                reg->nomeEstacao    = strdup(camposAtu[k].valueOfTheField);
            }
        }
        else if (strcmp(camposAtu[k].nameOfTheField, "nomeLinha") == 0) {
            if (reg->tamNomeLinha > 0) free(reg->nomeLinha); //libera a string antiga
            if (strcmp(camposAtu[k].valueOfTheField, "") == 0) {
                reg->tamNomeLinha = 0;
                reg->nomeLinha    = NULL;
            } else {
                reg->tamNomeLinha = strlen(camposAtu[k].valueOfTheField);
                reg->nomeLinha    = strdup(camposAtu[k].valueOfTheField);
            }
        }
    }
}

// 1. Verifica se o codEstacao é um dos filtros aplicados pelo usuário.
int obterCodEstacaoBusca(Field *campos, int m)
{
    for (int k = 0; k < m; k++) {
        if (strcmp(campos[k].nameOfTheField, "codEstacao") == 0)
            return (strcmp(campos[k].valueOfTheField, "") == 0) ? -1 : atoi(campos[k].valueOfTheField);
    }
    return -2; // Retorna -2 se o codEstacao não estiver nos filtros
}

// 2. Testa o registro atual contra todos os filtros (usada nas funções 7 e 9 dele)
int registroCorresponde(Registry *reg, Field *campos, int m)
{
    for (int k = 0; k < m; k++) {
        if (strcmp(campos[k].nameOfTheField, "codEstacao") == 0) {
            int val = (strcmp(campos[k].valueOfTheField, "") == 0) ? -1 : atoi(campos[k].valueOfTheField);
            if (reg->codEstacao != val) return 0;
        }
        else if (strcmp(campos[k].nameOfTheField, "codLinha") == 0) {
            int val = (strcmp(campos[k].valueOfTheField, "") == 0) ? -1 : atoi(campos[k].valueOfTheField);
            if (reg->codLinha != val) return 0;
        }
        else if (strcmp(campos[k].nameOfTheField, "codProxEstacao") == 0) {
            int val = (strcmp(campos[k].valueOfTheField, "") == 0) ? -1 : atoi(campos[k].valueOfTheField);
            if (reg->codProxEstacao != val) return 0;
        }
        else if (strcmp(campos[k].nameOfTheField, "distProxEstacao") == 0) {
            int val = (strcmp(campos[k].valueOfTheField, "") == 0) ? -1 : atoi(campos[k].valueOfTheField);
            if (reg->distProxEstacao != val) return 0;
        }
        else if (strcmp(campos[k].nameOfTheField, "codLinhaIntegra") == 0) {
            int val = (strcmp(campos[k].valueOfTheField, "") == 0) ? -1 : atoi(campos[k].valueOfTheField);
            if (reg->codLinhaIntegra != val) return 0;
        }
        else if (strcmp(campos[k].nameOfTheField, "codEstIntegra") == 0) {
            int val = (strcmp(campos[k].valueOfTheField, "") == 0) ? -1 : atoi(campos[k].valueOfTheField);
            if (reg->codEstIntegra != val) return 0;
        }
        else if (strcmp(campos[k].nameOfTheField, "nomeEstacao") == 0) {
            if (reg->tamNomeEstacao == 0) {
                if (strcmp(campos[k].valueOfTheField, "") != 0) return 0;
            } else {
                if (strcmp(reg->nomeEstacao, campos[k].valueOfTheField) != 0) return 0;
            }
        }
        else if (strcmp(campos[k].nameOfTheField, "nomeLinha") == 0) {
            if (reg->tamNomeLinha == 0) {
                if (strcmp(campos[k].valueOfTheField, "") != 0) return 0;
            } else {
                if (strcmp(reg->nomeLinha, campos[k].valueOfTheField) != 0) return 0;
            }
        }
    }
    return 1;
}

// 3. Lê dados brutos do terminal para inserir um registro novo (Funcionalidade 8)
void lerRegistroStdin(Registry *reg)
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