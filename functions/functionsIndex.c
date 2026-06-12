//functionsIndex.c
//implementa as funcionalidades 5 a 9 do tp1
//5 criar arquivo de indice primario a partir do binario de dados
//6 busca usando o indice quando possivel ou varredura sequencial
//7 remocao logica com pilha de reaproveitamento e atualizacao do indice
//8 insercao reutilizando espaco removido ou acrescentando ao final
//9 atualizacao inplace dos campos e correcao do indice se necessario
//obs: a verificacao de consistencia dos dois arquivos e feita em main.c
//antes de cada chamada, seguindo o mesmo padrao das funcionalidades 2 3 e 4

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

//par nome e valor de um campo usado para representar criterios de busca e atualizacao
typedef struct campo {
    char nome[100];  //nome do campo ex: codEstacao nomeLinha
    char valor[100]; //valor como string; campo nulo chega como string vazia ""
} Campo;

// =========================================================
// funcoes auxiliares internas (static = visiveis so aqui)
// =========================================================

//percorre todos os registros validos e conta:
//  nEst   -> quantos nomes de estacao distintos existem
//  nPares -> quantos pares (codEstacao, codProxEstacao) distintos existem
//isso e usado para calcular o quanto nroEstacoes e nroParesEstacao mudam
//apos uma insercao, remocao ou atualizacao (calculo via delta antes/depois)
static void contarUnicosValidos(FILE *arq, int *nEst, int *nPares)
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

//funcao de comparacao usada pelo qsort para ordenar entradas do indice
//subtrai os codEstacao: negativo = a vem antes, zero = iguais, positivo = b vem antes
static int comparaEntradaIndice(const void *a, const void *b)
{
    const Index *ia = (const Index *)a;
    const Index *ib = (const Index *)b;
    return ia->codEstacao - ib->codEstacao;
}

//le todos os 5 campos do cabecalho do arquivo de dados para a struct Header
static void lerCabecalho(FILE *arq, Header *cab)
{
    fseek(arq, 0, SEEK_SET);
    fread(&cab->status,          sizeof(char), 1, arq);
    fread(&cab->topo,            sizeof(int),  1, arq);
    fread(&cab->proxRRN,         sizeof(int),  1, arq);
    fread(&cab->nroEstacoes,     sizeof(int),  1, arq);
    fread(&cab->nroParesEstacao, sizeof(int),  1, arq);
}

//escreve os 5 campos da struct Header de volta no inicio do arquivo de dados
static void escreverCabecalho(FILE *arq, Header *cab)
{
    fseek(arq, 0, SEEK_SET);
    fwrite(&cab->status,          sizeof(char), 1, arq);
    fwrite(&cab->topo,            sizeof(int),  1, arq);
    fwrite(&cab->proxRRN,         sizeof(int),  1, arq);
    fwrite(&cab->nroEstacoes,     sizeof(int),  1, arq);
    fwrite(&cab->nroParesEstacao, sizeof(int),  1, arq);
}

//calcula quantas entradas de dados ha no arquivo de indice
//tamanho total menos 1 byte de cabecalho dividido por 8 bytes por entrada
static int contarEntradasIndice(FILE *arqIndice)
{
    fseek(arqIndice, 0, SEEK_END);
    long tamanho = ftell(arqIndice);
    return (int)((tamanho - sizeof(char)) / INDEX_SIZE);
}

//busca binaria no arquivo de indice pelo codEstacao
//o indice esta sempre ordenado crescentemente entao e possivel descartar metade a cada passo
//retorna o RRN do registro de dados correspondente ou -1 se nao encontrar
static int buscaBinariaIndice(FILE *arqIndice, int codEstacao)
{
    int total = contarEntradasIndice(arqIndice);
    int esq = 0, dir = total - 1;

    while (esq <= dir) {
        int meio = (esq + dir) / 2;

        //salta direto ate a entrada do meio: 1 byte de cabecalho + (meio * 8 bytes)
        fseek(arqIndice, sizeof(char) + (long)meio * INDEX_SIZE, SEEK_SET);

        Index entrada;
        fread(&entrada.codEstacao, sizeof(int), 1, arqIndice);
        fread(&entrada.RRN,        sizeof(int), 1, arqIndice);

        if (entrada.codEstacao == codEstacao)
            return entrada.RRN;          //achou: devolve o RRN
        else if (entrada.codEstacao < codEstacao)
            esq = meio + 1;              //procura na metade direita
        else
            dir = meio - 1;              //procura na metade esquerda
    }

    return -1; //nao encontrou
}

//remove do arquivo de indice a entrada com o codEstacao informado
//estrategia: carrega tudo na memoria, reescreve pulando a entrada removida,
//depois trunca o arquivo para o novo tamanho (sem isso sobra lixo no final)
static void removerDoIndice(FILE *arqIndice, int codEstacao)
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
static void inserirNoIndice(FILE *arqIndice, int codEstacao, int rrn)
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

//testa se um registro satisfaz todos os m criterios de busca passados
//campo inteiro com valor vazio "" e tratado como -1 (valor nulo no binario)
//retorna 1 se o registro bate com todos os criterios, 0 se falhar em qualquer um
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

//verifica se codEstacao esta entre os m criterios de busca
//retorna o valor inteiro buscado (ou -1 se o valor era nulo "")
//retorna -2 como sentinela especial quando codEstacao nao e criterio de busca
//  -2 significa: nao ha como usar o indice, faca varredura sequencial
static int obterCodEstacaoBusca(Campo *campos, int m)
{
    for (int k = 0; k < m; k++) {
        if (strcmp(campos[k].nome, "codEstacao") == 0)
            return (strcmp(campos[k].valor, "") == 0) ? -1 : atoi(campos[k].valor);
    }
    return -2; //codEstacao nao esta nos criterios de busca
}

//le do stdin os 8 campos de um novo registro na ordem do binario:
//codEstacao nomeEstacao codLinha nomeLinha codProxEstacao
//distProxEstacao codLinhaIntegra codEstIntegra
//ScanQuoteString trata aspas e converte "NULO" para string vazia ""
//campos inteiros vazios viram -1; campos string vazios ficam como NULL
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
        reg->nomeEstacao    = strdup(buf); //aloca copia da string
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

    //registro novo nunca esta removido e ainda nao tem proximo na pilha
    reg->removido = IS_NOT_REMOVED;
    reg->proximo  = -1;
}

//aplica os p pares (campo, novo valor) sobre o registro que esta em memoria
//para campos string: libera a string antiga antes de alocar a nova
//valor vazio "" vira -1 para inteiros e NULL para strings (campo nulo)
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
            if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao); //libera a string antiga
            if (strcmp(camposAtu[k].valor, "") == 0) {
                reg->tamNomeEstacao = 0;
                reg->nomeEstacao    = NULL;
            } else {
                reg->tamNomeEstacao = strlen(camposAtu[k].valor);
                reg->nomeEstacao    = strdup(camposAtu[k].valor);
            }
        }
        else if (strcmp(camposAtu[k].nome, "nomeLinha") == 0) {
            if (reg->tamNomeLinha > 0) free(reg->nomeLinha); //libera a string antiga
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

// =========================================================
// funcionalidade 5: criar arquivo de indice primario
// =========================================================
//le o arquivo de dados inteiro e coleta os pares (codEstacao, RRN) de
//todos os registros nao removidos, ordena crescentemente por codEstacao
//e grava no arquivo de indice
//estrutura do indice: 1 byte status + N entradas de 8 bytes cada (4 codEstacao + 4 RRN)
int createPrimaryIndexArchive(FILE *registryBinaryFile, FILE *primaryIndexArchive)
{
    if (registryBinaryFile == NULL || primaryIndexArchive == NULL)
        return FUNCTION_FAILURE;

    //marca o indice como inconsistente ('0') enquanto estamos construindo ele
    //se o programa cair aqui o arquivo fica marcado como invalido
    fseek(primaryIndexArchive, 0, SEEK_SET);
    char statusIndice = INDEX_INCONSISTENT;
    fwrite(&statusIndice, sizeof(char), 1, primaryIndexArchive);

    //aloca vetor temporario para guardar os pares antes de ordenar
    int capacidade = 10000;
    Index *entradas = malloc(capacidade * sizeof(Index));
    int total = 0;

    //posiciona apos o cabecalho de 17 bytes e percorre os registros um a um
    fseek(registryBinaryFile, HEADER_SIZE, SEEK_SET);
    Registry reg;
    int rrn = 0; //RRN = posicao sequencial do registro no arquivo (0, 1, 2...)

    while (binaryToRegistry(&reg, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS) {
        //registros removidos logicamente nao entram no indice
        if (reg.removido == IS_NOT_REMOVED) {
            entradas[total].codEstacao = reg.codEstacao;
            entradas[total].RRN        = rrn;
            total++;
        }
        freeRegistry(&reg);
        rrn++;
    }

    //ordena os pares coletados por codEstacao crescente usando qsort
    qsort(entradas, total, sizeof(Index), comparaEntradaIndice);

    //grava as entradas ja ordenadas no arquivo de indice campo por campo
    for (int i = 0; i < total; i++) {
        fwrite(&entradas[i].codEstacao, sizeof(int), 1, primaryIndexArchive);
        fwrite(&entradas[i].RRN,        sizeof(int), 1, primaryIndexArchive);
    }

    //tudo gravado com sucesso: marca o indice como consistente ('1')
    fseek(primaryIndexArchive, 0, SEEK_SET);
    statusIndice = INDEX_CONSISTENT;
    fwrite(&statusIndice, sizeof(char), 1, primaryIndexArchive);

    free(entradas);
    return FUNCTION_SUCESS;
}

// =========================================================
// funcionalidade 6: busca usando o indice primario
// =========================================================
//para cada uma das numberOfSearches buscas independentes:
//  se codEstacao estiver nos criterios: usa busca binaria no indice (O log n)
//    o indice devolve o RRN e vamos direto ao slot no arquivo de dados
//  caso contrario: varredura sequencial em todo o arquivo de dados
//imprime cada registro encontrado ou "Registro inexistente." se nao achar nenhum
int restoreIndexArchive(FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches)
{
    if (registryBinaryFile == NULL || primaryIndexArchive == NULL)
        return FUNCTION_FAILURE;

    for (int i = 0; i < numberOfSearches; i++) {
        //le quantos pares campo/valor compoe esta busca especifica
        int m;
        scanf("%d", &m);

        Campo campos[m];
        for (int j = 0; j < m; j++) {
            scanf("%s", campos[j].nome);
            ScanQuoteString(campos[j].valor);
        }

        int achouPeloMenosUm = 0;

        //obterCodEstacaoBusca retorna -2 se codEstacao nao esta nos criterios
        int codBuscado = obterCodEstacaoBusca(campos, m);

        if (codBuscado != -2) {
            //busca indexada: busca binaria no indice retorna o RRN diretamente
            int rrn = buscaBinariaIndice(primaryIndexArchive, codBuscado);
            if (rrn >= 0) {
                //calcula o offset: cabecalho + rrn * 80 bytes por registro
                fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                Registry reg;
                if (binaryToRegistry(&reg, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS) {
                    //mesmo com o indice confirma que o registro nao foi removido
                    //e que satisfaz os outros filtros alem do codEstacao
                    if (reg.removido == IS_NOT_REMOVED && registroCorresponde(&reg, campos, m)) {
                        printRegistry(&reg);
                        achouPeloMenosUm = 1;
                    }
                    freeRegistry(&reg);
                }
            }
        } else {
            //busca sequencial: le todos os registros do inicio ao fim
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

        //linha em branco separando a saida de buscas consecutivas (exceto apos a ultima)
        if (i < numberOfSearches - 1)
            printf("\n");
    }

    return FUNCTION_SUCESS;
}

// =========================================================
// funcionalidade 7: remocao logica com pilha e indice
// =========================================================
//para cada uma das numberOfSearches operacoes de remocao:
//  encontra os registros que batem com o criterio (indexada ou sequencial)
//  aplica remocao logica: marca removido='1' e empilha na pilha de reaproveitamento
//    a pilha e implementada no campo "proximo" do registro e "topo" do cabecalho
//  remove a entrada correspondente do arquivo de indice
//ao final atualiza o cabecalho (topo, nroEstacoes, nroParesEstacao) e marca consistente
int removeIndexArchive(FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches)
{
    if (registryBinaryFile == NULL || primaryIndexArchive == NULL)
        return FUNCTION_FAILURE;

    //marca ambos os arquivos como inconsistentes antes de comecar a alterar
    //se o programa cair no meio da operacao os arquivos ficam sinalizados como invalidos
    char inconsistente = STATUS_INCONSISTENT;
    fseek(registryBinaryFile, 0, SEEK_SET);
    fwrite(&inconsistente, sizeof(char), 1, registryBinaryFile);

    char inconsistenteIndice = INDEX_INCONSISTENT;
    fseek(primaryIndexArchive, 0, SEEK_SET);
    fwrite(&inconsistenteIndice, sizeof(char), 1, primaryIndexArchive);

    //le o cabecalho para acessar o topo atual da pilha de registros removidos
    Header cab;
    lerCabecalho(registryBinaryFile, &cab);

    //snapshot dos contadores antes das remocoes para calcular o delta ao final
    int estAntes, paresAntes;
    contarUnicosValidos(registryBinaryFile, &estAntes, &paresAntes);

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
            //busca indexada: vai direto ao RRN pelo indice
            int rrn = buscaBinariaIndice(primaryIndexArchive, codBuscado);
            if (rrn >= 0) {
                fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                Registry reg;
                if (binaryToRegistry(&reg, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS) {
                    if (reg.removido == IS_NOT_REMOVED && registroCorresponde(&reg, campos, m)) {
                        int cod = reg.codEstacao;
                        freeRegistry(&reg); //libera antes de reposicionar o cursor

                        //sobrescreve apenas os primeiros 5 bytes do registro:
                        //  byte 0: removido = '1' (marca como deletado)
                        //  bytes 1-4: proximo = topo atual (encadeia na pilha)
                        //os demais 75 bytes permanecem inalterados (dados ficam la)
                        fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                        char marcaRemovido = IS_REMOVED;
                        fwrite(&marcaRemovido, sizeof(char), 1, registryBinaryFile);
                        fwrite(&cab.topo,      sizeof(int),  1, registryBinaryFile);
                        cab.topo = rrn; //esse RRN e o novo topo da pilha

                        removerDoIndice(primaryIndexArchive, cod);
                    } else {
                        freeRegistry(&reg);
                    }
                }
            }
        } else {
            //busca sequencial: posiciona no inicio e avanca registro a registro
            //fseek adicional so ocorre quando encontramos e removemos um registro
            fseek(registryBinaryFile, HEADER_SIZE, SEEK_SET);
            int rrn = 0;
            while (rrn < cab.proxRRN) { //proxRRN e o total de slots usados (incluindo removidos)
                Registry reg;
                if (binaryToRegistry(&reg, registryBinaryFile) != BINARY_TO_REGISTRY_SUCESS)
                    break;

                if (reg.removido == IS_NOT_REMOVED && registroCorresponde(&reg, campos, m)) {
                    int cod = reg.codEstacao;
                    freeRegistry(&reg);

                    //volta ao inicio deste registro para sobrescrever removido e proximo
                    fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                    char marcaRemovido = IS_REMOVED;
                    fwrite(&marcaRemovido, sizeof(char), 1, registryBinaryFile);
                    fwrite(&cab.topo,      sizeof(int),  1, registryBinaryFile);
                    cab.topo = rrn;

                    //escrevemos 5 bytes (1 char + 1 int); avanca os 75 restantes
                    //para deixar o cursor no inicio do proximo registro
                    fseek(registryBinaryFile, REGISTRY_SIZE - sizeof(char) - sizeof(int), SEEK_CUR);

                    removerDoIndice(primaryIndexArchive, cod);
                } else {
                    freeRegistry(&reg);
                    //binaryToRegistry ja deixou o cursor no inicio do proximo registro
                }
                rrn++;
            }
        }
    }

    //calcula quantos unicos existem agora e aplica a diferenca nos contadores do cabecalho
    {
        int estDepois, paresDepois;
        contarUnicosValidos(registryBinaryFile, &estDepois, &paresDepois);
        cab.nroEstacoes     += estDepois   - estAntes;
        cab.nroParesEstacao += paresDepois - paresAntes;
    }

    //grava o cabecalho atualizado e marca o arquivo de dados como consistente
    cab.status = STATUS_CONSISTENT;
    escreverCabecalho(registryBinaryFile, &cab);

    //marca o arquivo de indice como consistente
    fseek(primaryIndexArchive, 0, SEEK_SET);
    char consistenteIndice = INDEX_CONSISTENT;
    fwrite(&consistenteIndice, sizeof(char), 1, primaryIndexArchive);

    return FUNCTION_SUCESS;
}

// =========================================================
// funcionalidade 8: insercao com reaproveitamento e indice
// =========================================================
//para cada um dos numberOfRegistries novos registros:
//  se a pilha de espacos livres nao estiver vazia (topo != -1):
//    reutiliza o slot do topo, le o campo "proximo" para desempilhar
//  caso contrario:
//    acrescenta ao final do arquivo incrementando proxRRN
//  grava o registro novo na posicao calculada e insere no indice
//ao final atualiza cabecalho (topo, proxRRN, nroEstacoes, nroParesEstacao)
int insertNewIndexArchive(FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfRegistries)
{
    if (registryBinaryFile == NULL || primaryIndexArchive == NULL)
        return FUNCTION_FAILURE;

    //marca ambos os arquivos como inconsistentes antes de alterar
    char inconsistente = STATUS_INCONSISTENT;
    fseek(registryBinaryFile, 0, SEEK_SET);
    fwrite(&inconsistente, sizeof(char), 1, registryBinaryFile);

    char inconsistenteIndice = INDEX_INCONSISTENT;
    fseek(primaryIndexArchive, 0, SEEK_SET);
    fwrite(&inconsistenteIndice, sizeof(char), 1, primaryIndexArchive);

    //le o cabecalho para saber o topo da pilha e o proximo RRN disponivel
    Header cab;
    lerCabecalho(registryBinaryFile, &cab);

    //snapshot dos contadores antes das insercoes para calcular o delta ao final
    int estAntes, paresAntes;
    contarUnicosValidos(registryBinaryFile, &estAntes, &paresAntes);

    for (int i = 0; i < numberOfRegistries; i++) {
        Registry reg;
        lerRegistroStdin(&reg); //le os 8 campos do novo registro do stdin

        int rrnNovo;

        if (cab.topo != -1) {
            //ha espaco livre na pilha: reutiliza o slot apontado pelo topo
            rrnNovo = cab.topo;

            //le o campo "proximo" do slot removido para descobrir o proximo da pilha
            //proximo fica nos bytes 1 a 4 do registro (logo apos o byte "removido")
            fseek(registryBinaryFile,
                  HEADER_SIZE + (long)rrnNovo * REGISTRY_SIZE + sizeof(char),
                  SEEK_SET);
            fread(&cab.topo, sizeof(int), 1, registryBinaryFile); //desempilha
        } else {
            //pilha vazia: acrescenta ao final do arquivo e avanca proxRRN
            rrnNovo = cab.proxRRN;
            cab.proxRRN++;
        }

        //salva o codEstacao antes de registryToBinary pois ela libera os ponteiros da struct
        int codEstacaoNovo = reg.codEstacao;

        //escreve os 80 bytes do novo registro na posicao calculada
        fseek(registryBinaryFile, HEADER_SIZE + (long)rrnNovo * REGISTRY_SIZE, SEEK_SET);
        registryToBinary(&reg, registryBinaryFile);

        //insere o par (codEstacao, rrnNovo) no indice mantendo a ordem crescente
        inserirNoIndice(primaryIndexArchive, codEstacaoNovo, rrnNovo);
    }

    //calcula o delta de unicos e aplica nos contadores do cabecalho
    {
        int estDepois, paresDepois;
        contarUnicosValidos(registryBinaryFile, &estDepois, &paresDepois);
        cab.nroEstacoes     += estDepois   - estAntes;
        cab.nroParesEstacao += paresDepois - paresAntes;
    }

    //grava o cabecalho com topo e proxRRN atualizados e marca como consistente
    cab.status = STATUS_CONSISTENT;
    escreverCabecalho(registryBinaryFile, &cab);

    //marca o arquivo de indice como consistente
    fseek(primaryIndexArchive, 0, SEEK_SET);
    char consistenteIndice = INDEX_CONSISTENT;
    fwrite(&consistenteIndice, sizeof(char), 1, primaryIndexArchive);

    return FUNCTION_SUCESS;
}

// =========================================================
// funcionalidade 9: atualizacao inplace com manutencao do indice
// =========================================================
//para cada uma das numberOfUpdates operacoes de atualizacao:
//  le m pares (campo, valor) de busca e p pares (campo, novo valor) de atualizacao
//  encontra os registros que batem com a busca (indexada ou sequencial)
//  aplica as alteracoes diretamente nos 80 bytes do slot (inplace, sem mover o registro)
//  se codEstacao mudou: remove a entrada antiga do indice e insere a nova
//ao final atualiza os contadores do cabecalho e marca os arquivos como consistentes
int updateIndexArchive(FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfUpdates)
{
    if (registryBinaryFile == NULL || primaryIndexArchive == NULL)
        return FUNCTION_FAILURE;

    //marca ambos os arquivos como inconsistentes antes de alterar
    char inconsistente = STATUS_INCONSISTENT;
    fseek(registryBinaryFile, 0, SEEK_SET);
    fwrite(&inconsistente, sizeof(char), 1, registryBinaryFile);

    char inconsistenteIndice = INDEX_INCONSISTENT;
    fseek(primaryIndexArchive, 0, SEEK_SET);
    fwrite(&inconsistenteIndice, sizeof(char), 1, primaryIndexArchive);

    //le o cabecalho para acessar proxRRN (total de slots ocupados no arquivo)
    Header cab;
    lerCabecalho(registryBinaryFile, &cab);

    //snapshot dos contadores antes das atualizacoes para calcular o delta ao final
    int estAntes, paresAntes;
    contarUnicosValidos(registryBinaryFile, &estAntes, &paresAntes);

    for (int i = 0; i < numberOfUpdates; i++) {
        //le os m criterios de busca (campo + valor a comparar)
        int m;
        scanf("%d", &m);
        Campo camposBusca[m];
        for (int j = 0; j < m; j++) {
            scanf("%s", camposBusca[j].nome);
            ScanQuoteString(camposBusca[j].valor);
        }

        //le os p campos a atualizar (campo + novo valor)
        int p;
        scanf("%d", &p);
        Campo camposAtu[p];
        for (int j = 0; j < p; j++) {
            scanf("%s", camposAtu[j].nome);
            ScanQuoteString(camposAtu[j].valor);
        }

        int codBuscado = obterCodEstacaoBusca(camposBusca, m);

        if (codBuscado != -2) {
            //busca indexada: localiza o RRN pelo indice e vai direto ao slot
            int rrn = buscaBinariaIndice(primaryIndexArchive, codBuscado);
            if (rrn >= 0) {
                fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                Registry reg;
                if (binaryToRegistry(&reg, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS) {
                    if (reg.removido == IS_NOT_REMOVED && registroCorresponde(&reg, camposBusca, m)) {
                        int codAntigo = reg.codEstacao;

                        //aplica as alteracoes sobre o registro que esta em memoria
                        aplicarAtualizacoes(&reg, camposAtu, p);

                        int codNovo = reg.codEstacao;

                        //reescreve os 80 bytes na mesma posicao (atualizacao inplace)
                        fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                        registryToBinary(&reg, registryBinaryFile);

                        //se o codEstacao mudou: remove a entrada antiga do indice
                        //e insere a nova com o mesmo RRN mas o novo codigo
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
            //busca sequencial: percorre todos os slots do arquivo um a um
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

                    //volta ao inicio deste slot e reescreve os 80 bytes (inplace)
                    //registryToBinary escreve exatamente REGISTRY_SIZE bytes, entao
                    //o cursor fica automaticamente no inicio do proximo slot
                    fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                    registryToBinary(&reg, registryBinaryFile);

                    if (codNovo != codAntigo) {
                        removerDoIndice(primaryIndexArchive, codAntigo);
                        inserirNoIndice(primaryIndexArchive, codNovo, rrn);
                    }
                } else {
                    freeRegistry(&reg);
                    //binaryToRegistry ja posicionou o cursor no proximo slot
                }
                rrn++;
            }
        }
    }

    //calcula o delta de unicos e aplica nos contadores do cabecalho
    {
        int estDepois, paresDepois;
        contarUnicosValidos(registryBinaryFile, &estDepois, &paresDepois);
        cab.nroEstacoes     += estDepois   - estAntes;
        cab.nroParesEstacao += paresDepois - paresAntes;
    }

    //grava o cabecalho atualizado e marca o arquivo de dados como consistente
    cab.status = STATUS_CONSISTENT;
    escreverCabecalho(registryBinaryFile, &cab);

    //marca o arquivo de indice como consistente
    fseek(primaryIndexArchive, 0, SEEK_SET);
    char consistenteIndice = INDEX_CONSISTENT;
    fwrite(&consistenteIndice, sizeof(char), 1, primaryIndexArchive);

    return FUNCTION_SUCESS;
}
