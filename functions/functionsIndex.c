#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "index.h"
#include "functionsIndex.h"
#include "utils.h"
#include "header.h"
#include "registry.h"
#include "functions.h"

/* Crie um arquivo de índice primário para um arquivo de dados de entrada. O arquivo
de índice primário deve ser gerado de acordo com as especificações da Descrição do
Arquivo de Índice Primário definidas neste trabalho prático, e deve indexar
corretamente o arquivo de dados criado na funcionalidade [1]. Isso significa que, no
momento da criação, o arquivo de índice primário deve possuir um registro de cabeçalho
e vários registros de dados*/

int createPrimaryIndexArchiveInBinary( FILE *registryBinaryFile, FILE *primaryIndexArchive)
{
    if(registryBinaryFile == NULL || primaryIndexArchive == NULL){
        printf("Falha no processamento do arquivo."); // Error handling
        return FUNCTION_FAILURE;
    }

    char indexConsistency = INDEX_INCONSISTENT;
    fwrite(&indexConsistency, sizeof(char), 1, primaryIndexArchive); // defines consistency

    Index unorderedIndex[5000];
    Registry temporaryRegistry;

    int rrn = 0;

    int numberOfRegisters = 0; // total of registers
    int numberOfValidRegisters = 0;

    int comparison = 0;

    fseek(registryBinaryFile, 17, SEEK_SET);

    while(binaryToRegistry(&temporaryRegistry, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS)
    { // while registers exist, read it and save it on RAM


        if(temporaryRegistry.removido == IS_NOT_REMOVED)
        {
            unorderedIndex[numberOfValidRegisters].codEstacao = temporaryRegistry.codEstacao;
            unorderedIndex[numberOfValidRegisters].RRN = numberOfRegisters;
            numberOfValidRegisters++;
        }

        freeRegistry(&temporaryRegistry);
        numberOfRegisters++;
    }

    qsort(unorderedIndex, numberOfValidRegisters, sizeof(Index), compareCodEstacao);
    // the index needs to be in crescent order, so we call qsort

    //after ordered, now we write it in order on the definitive file.
    // We have now an ordered array

    while(comparison < numberOfValidRegisters)
    {
        fwrite(&unorderedIndex[comparison].codEstacao, sizeof(int), 1, primaryIndexArchive);
        fwrite(&unorderedIndex[comparison].RRN, sizeof(int), 1, primaryIndexArchive);

        comparison++;
    } /* The document, ordered and complete, was written.*/

    indexConsistency = INDEX_CONSISTENT;

    fseek(primaryIndexArchive, 0, SEEK_SET);
    fwrite(&indexConsistency, sizeof(char), 1, primaryIndexArchive);

    return FUNCTION_SUCESS;
}

/*
    [6] Permita a recuperação dos dados de todos os registros de um arquivo de dados de
entrada, de forma que esses registros satisfaçam um critério de busca determinado pelo
usuário. Qualquer campo pode ser utilizado como forma de busca. Adicionalmente, a
busca deve ser feita considerando um ou mais campos. Por exemplo, é possível realizar
a busca considerando somente o campo codEstacao ou considerando os campos
nomeEstacao e nomeLinha. Em situações nas quais a busca for feita considerando o
campo codEstacao, deve ser utilizado o arquivo de índice indexaEstacao para se fazer
a busca. Tem-se, nesse caso, uma busca indexada. Para os demais casos, deve ser feita
uma busca sequencial. Esta funcionalidade pode retornar 0 registros (quando nenhum
satisfaz ao critério de busca), 1 registro (quando apenas um satisfaz ao critério de busca),
ou vários registros. Os valores dos campos do tipo string devem ser especificados entre
aspas duplas ("). Para a manipulação de strings com aspas duplas, pode-se usar a função
scan_quote_string disponibilizada na página do projeto da disciplina. Para a busca por
campos nulos, deve-se especificar o valor NULO. Registros marcados como
logicamente removidos não devem ser exibidos. O arquivo de dados de entrada deve ser
percorrido apropriadamente. 
*/

int searchOnIndexArchive( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches)
{

    if(registryBinaryFile == NULL || primaryIndexArchive == NULL)
    {
        printf("Falha no processamento do arquivo."); // Error handling
        return FUNCTION_FAILURE;
    }
    

    /* numberOfSearches will be the number of searches made. (this is the first loop, which emcomprises the other two loops).
    * numberOfFiltersApplied will be the filters applied to each search. (this is the second loop).
    * searchForEachFilter loops through each filter and verifies it. (the second loop)
    * */
    int numberOfFiltersApplied = 0; // Number of filters in each search! given as INPUT by the user.

    for(int quantityOfSearches = 0; quantityOfSearches < numberOfSearches; quantityOfSearches++)
    {
        /* loop responsible for the NUMBER of SEARCHES.*/
        int registersThatFulfillTheSearch = 0; // Number of registers that match the search, we will return it in the end!

        scanf("%d", &numberOfFiltersApplied);

        Field fieldsToBeSearched[numberOfFiltersApplied];
        Registry temporaryRegister;

        for (int numberOfFilters = 0; numberOfFilters < numberOfFiltersApplied; numberOfFilters++)
        {
            /* Loop responsible for collecting the n filters we want to aplly to THIS search*/
            scanf("%s", fieldsToBeSearched[numberOfFilters].nameOfTheField);
            ScanQuoteString(fieldsToBeSearched[numberOfFilters].valueOfTheField);
        }

        int indexCodEstacao = -1;
        for(int f = 0; f < numberOfFiltersApplied; f++)
        {
            if(strcmp(fieldsToBeSearched[f].nameOfTheField, "codEstacao") == 0)
            {
                indexCodEstacao = f;
            } // checks for codEstacao
        }

        if(indexCodEstacao != -1) // If we want to search for codEstacao, we use an indexed search.
        {
            int value = atoi(fieldsToBeSearched[indexCodEstacao].valueOfTheField);
                 
            fseek(primaryIndexArchive, 0, SEEK_END);

            long indexByteSize = (ftell(primaryIndexArchive) - 1) / sizeof(Index);
            //getting the whole size of the data

            Index *indexArray = (Index *) malloc((sizeof(Index)) * indexByteSize);
            // allocating in memory

            fseek(primaryIndexArchive, 1, SEEK_SET);
            fread(indexArray, sizeof(Index), indexByteSize, primaryIndexArchive);
            // moving the cursor and reading it all

            int foundRRN = binarySearchOnIndex(indexArray, indexByteSize, value);

            free(indexArray);

            if (foundRRN != -1)
            {
                int byteOffset = HEADER_SIZE + (foundRRN * 80);
                fseek(registryBinaryFile, byteOffset, SEEK_SET);

               singleSearchInRegister(temporaryRegister, registryBinaryFile, fieldsToBeSearched, numberOfFiltersApplied, &registersThatFulfillTheSearch);
            }
        }

        else{ //else, use the sequential search previously used on function searchData

        fseek(registryBinaryFile, HEADER_SIZE, SEEK_SET);

        sequentialSearchInRegister(temporaryRegister, registryBinaryFile, numberOfFiltersApplied, fieldsToBeSearched, &registersThatFulfillTheSearch);

        }

        if (registersThatFulfillTheSearch == 0)
        {
            printf("Registro inexistente.\n");
        }

        if(quantityOfSearches < numberOfSearches - 1)
        {
            printf("\n");
        }
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
    readHeader(registryBinaryFile, &cab);

    //snapshot dos contadores antes das remocoes para calcular o delta ao final
    int estAntes, paresAntes;
    countValidRecords(registryBinaryFile, &estAntes, &paresAntes);

    for (int i = 0; i < numberOfSearches; i++) {
        int m;
        scanf("%d", &m);

        Field campos[m];
        for (int j = 0; j < m; j++) {
            scanf("%s", campos[j].nameOfTheField);
            ScanQuoteString(campos[j].valueOfTheField);
        }

        int codBuscado = getCodEstacaoForSearch(campos, m);

        if (codBuscado != -2) {
            //busca indexada: vai direto ao RRN pelo indice
            fseek(primaryIndexArchive, 0, SEEK_END);
            long indexByteSize = (ftell(primaryIndexArchive) - 1) / sizeof(Index);
            Index *indexArray = (Index *) malloc(sizeof(Index) * indexByteSize);

            fseek(primaryIndexArchive, 1, SEEK_SET);
            fread(indexArray, sizeof(Index), indexByteSize, primaryIndexArchive);

            int rrn = binarySearchOnIndex(indexArray, indexByteSize, codBuscado);
            free(indexArray); // Libera a RAM

            if (rrn != -1) {
                if (rrn >= 0) {
                    fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                    Registry reg;
                    if (binaryToRegistry(&reg, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS) {
                        if (reg.removido == IS_NOT_REMOVED && isTheRegistryCorrespondent(&reg, campos, m)) {
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

                            removeByIndex(primaryIndexArchive, cod);
                        } else {
                            freeRegistry(&reg);
                        }
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

                if (reg.removido == IS_NOT_REMOVED && isTheRegistryCorrespondent(&reg, campos, m)) {
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

                    removeByIndex(primaryIndexArchive, cod);
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
        countValidRecords(registryBinaryFile, &estDepois, &paresDepois);
        cab.nroEstacoes     += estDepois   - estAntes;
        cab.nroParesEstacao += paresDepois - paresAntes;
    }

    //grava o cabecalho atualizado e marca o arquivo de dados como consistente
    cab.status = STATUS_CONSISTENT;
    writeHeader(registryBinaryFile, &cab);

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
    readHeader(registryBinaryFile, &cab);

    //snapshot dos contadores antes das insercoes para calcular o delta ao final
    int estAntes, paresAntes;
    countValidRecords(registryBinaryFile, &estAntes, &paresAntes);

    for (int i = 0; i < numberOfRegistries; i++) {
        Registry reg;
        readRegistryStdin(&reg); //le os 8 campos do novo registro do stdin

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
        insertOnIndex(primaryIndexArchive, codEstacaoNovo, rrnNovo);
    }

    //calcula o delta de unicos e aplica nos contadores do cabecalho
    {
        int estDepois, paresDepois;
        countValidRecords(registryBinaryFile, &estDepois, &paresDepois);
        cab.nroEstacoes     += estDepois   - estAntes;
        cab.nroParesEstacao += paresDepois - paresAntes;
    }

    //grava o cabecalho com topo e proxRRN atualizados e marca como consistente
    cab.status = STATUS_CONSISTENT;
    writeHeader(registryBinaryFile, &cab);

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
    readHeader(registryBinaryFile, &cab);

    //snapshot dos contadores antes das atualizacoes para calcular o delta ao final
    int estAntes, paresAntes;
    countValidRecords(registryBinaryFile, &estAntes, &paresAntes);

    for (int i = 0; i < numberOfUpdates; i++) {
        //le os m criterios de busca (campo + valor a comparar)
        int m;
        scanf("%d", &m);
        Field camposBusca[m];
        for (int j = 0; j < m; j++) {
            scanf("%s", camposBusca[j].nameOfTheField);
            ScanQuoteString(camposBusca[j].valueOfTheField);
        }

        //le os p campos a atualizar (campo + novo valor)
        int p;
        scanf("%d", &p);
        Field camposAtu[p];
        for (int j = 0; j < p; j++) {
            scanf("%s", camposAtu[j].nameOfTheField);
            ScanQuoteString(camposAtu[j].valueOfTheField);
        }

        int codBuscado = getCodEstacaoForSearch(camposBusca, m);

        if (codBuscado != -2) {
            //busca indexada: localiza o RRN pelo indice e vai direto ao slot
            fseek(primaryIndexArchive, 0, SEEK_END);
            long indexByteSize = (ftell(primaryIndexArchive) - 1) / sizeof(Index);
            Index *indexArray = (Index *) malloc(sizeof(Index) * indexByteSize);

            fseek(primaryIndexArchive, 1, SEEK_SET);
            fread(indexArray, sizeof(Index), indexByteSize, primaryIndexArchive);

            int rrn = binarySearchOnIndex(indexArray, indexByteSize, codBuscado);
            free(indexArray); // Libera a RAM

            if (rrn != -1) {
                fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                Registry reg;
                if (binaryToRegistry(&reg, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS) {
                    if (reg.removido == IS_NOT_REMOVED && isTheRegistryCorrespondent(&reg, camposBusca, m)) {
                        int codAntigo = reg.codEstacao;

                        //aplica as alteracoes sobre o registro que esta em memoria
                        updateRecords(&reg, camposAtu, p);

                        int codNovo = reg.codEstacao;

                        //reescreve os 80 bytes na mesma posicao (atualizacao inplace)
                        fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                        registryToBinary(&reg, registryBinaryFile);

                        //se o codEstacao mudou: remove a entrada antiga do indice
                        //e insere a nova com o mesmo RRN mas o novo codigo
                        if (codNovo != codAntigo) {
                            removeByIndex(primaryIndexArchive, codAntigo);
                            insertOnIndex(primaryIndexArchive, codNovo, rrn);
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

                if (reg.removido == IS_NOT_REMOVED && isTheRegistryCorrespondent(&reg, camposBusca, m)) {
                    int codAntigo = reg.codEstacao;

                    updateRecords(&reg, camposAtu, p);

                    int codNovo = reg.codEstacao;

                    //volta ao inicio deste slot e reescreve os 80 bytes (inplace)
                    //registryToBinary escreve exatamente REGISTRY_SIZE bytes, entao
                    //o cursor fica automaticamente no inicio do proximo slot
                    fseek(registryBinaryFile, HEADER_SIZE + (long)rrn * REGISTRY_SIZE, SEEK_SET);
                    registryToBinary(&reg, registryBinaryFile);

                    if (codNovo != codAntigo) {
                        removeByIndex(primaryIndexArchive, codAntigo);
                        insertOnIndex(primaryIndexArchive, codNovo, rrn);
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
        countValidRecords(registryBinaryFile, &estDepois, &paresDepois);
        cab.nroEstacoes     += estDepois   - estAntes;
        cab.nroParesEstacao += paresDepois - paresAntes;
    }

    //grava o cabecalho atualizado e marca o arquivo de dados como consistente
    cab.status = STATUS_CONSISTENT;
    writeHeader(registryBinaryFile, &cab);

    //marca o arquivo de indice como consistente
    fseek(primaryIndexArchive, 0, SEEK_SET);
    char consistenteIndice = INDEX_CONSISTENT;
    fwrite(&consistenteIndice, sizeof(char), 1, primaryIndexArchive);

    return FUNCTION_SUCESS;
}
