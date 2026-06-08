#include<stdio.h>

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

void createPrimaryIndexArchiveInBinary( FILE *registryBinaryFile, FILE *primaryIndexArchive)
{
    if(registryBinaryFile == NULL || primaryIndexArchive == NULL){
        printf("Falha no processamento do arquivo."); // Error handling
        return;
    }

    char indexConsistency = INDEX_INCONSISTENT;
    fwrite(&indexConsistency, sizeof(char), 1, primaryIndexArchive); // defines consistency

    Index unorderedIndex[5000];
    Registry temporaryRegistry;

    int rrn = 0;

    int numberOfRegisters = 0;
    int numberOfValidRegisters = 0;

    int comparison = 0;

    fseek(registryBinaryFile, 17, SEEK_SET);

    while(binaryToRegistry(&temporaryRegistry, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS)
    {


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

    //after ordered, now we write it in order on the definitive file.
    // We have now an orderer array

    while(comparison < numberOfValidRegisters)
    {
        fwrite(&unorderedIndex[comparison].codEstacao, sizeof(int), 1, primaryIndexArchive);
        fwrite(&unorderedIndex[comparison].RRN, sizeof(int), 1, primaryIndexArchive);

        comparison++;
    } /* The document, ordered and complete, was written.*/

    indexConsistency = INDEX_CONSISTENT;

    fseek(primaryIndexArchive, 0, SEEK_SET);
    fwrite(&indexConsistency, sizeof(char), 1, primaryIndexArchive);
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

    int registersThatFulfillTheSearch = 0; // Number of registers that match the search, we will return it in the end!
    int numberOfFiltersApplied = 0; // Number of filters in each search! given as INPUT by the user.

    for(int quantityOfSearches = 0; quantityOfSearches < numberOfSearches; quantityOfSearches++)
    {
        /* loop responsible for the NUMBER of SEARCHES.*/

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
            }
        }

        if(indexCodEstacao != -1)
        {
            int value = atoi(fieldsToBeSearched[indexCodEstacao].valueOfTheField);
                 
            fseek(primaryIndexArchive, 0, SEEK_END);

            long indexByteSize = (ftell(primaryIndexArchive) - 1) / sizeof(Index);

            Index *indexArray = (Index *) malloc((sizeof(Index)) * indexByteSize);

            fseek(primaryIndexArchive, 1, SEEK_SET);
            fread(indexArray, sizeof(Index), indexByteSize, primaryIndexArchive);

            int foundRRN = binarySearchOnIndex(indexArray, indexByteSize, value);

            free(indexArray);

            if (foundRRN != -1)
            {
                int byteOffset = HEADER_SIZE + (foundRRN * 80);
                fseek(registryBinaryFile, byteOffset, SEEK_SET);

                singleSearchInRegister(temporaryRegister, registryBinaryFile, numberOfFiltersApplied, fieldsToBeSearched, &registersThatFulfillTheSearch);
            }
        }

        else{

        fseek(registryBinaryFile, HEADER_SIZE, SEEK_SET);

        sequentialSearchInRegister(temporaryRegister, registryBinaryFile, numberOfFiltersApplied, fieldsToBeSearched, &registersThatFulfillTheSearch);

        }
    }

    return registersThatFulfillTheSearch;
}

void removeIndexArchive ( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches )
{
    ;
}

void insertNewIndexArchive ( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfRegistries )
{
    ;
}

