#include<stdio.h>

#include "index.h"
#include "functionsIndex.h"
#include "utils.h"
#include "header.h"
#include "registry.h"

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
        return;
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

        for (int numberOfFilters = 0; numberOfFilters < numberOfFiltersApplied; numberOfFilters++)
        {
            /* Loop responsible for collecting the n filters we want to aplly to THIS search*/
            scanf("%s", fieldsToBeSearched[numberOfFilters].nameOfTheField);
            ScanQuoteString(fieldsToBeSearched[numberOfFilters].valueOfTheField);
        }




        Registry temporaryRegister;
        int searchSucess = SEARCH_SUCESS;

        while(binaryToRegistry(&temporaryRegister, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS)
        {


            for (int searchForEachFilter = 0; searchForEachFilter < numberOfFiltersApplied; searchForEachFilter++)
            {

                if(strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "codEstacao") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    
                    fopen(primaryIndexArchive, "rb");
                    fseek(primaryIndexArchive, 0, SEEK_END);

                    long indexByteSize = (ftell(primaryIndexArchive) - 1) / 8;
                    Index *indexArray = (Index *) malloc((sizeof(Index)) * indexByteSize);

                    fread(indexArray, sizeof(Index), indexByteSize, primaryIndexArchive);

                    int foundRRN = binarySearchOnIndex(indexArray, indexByteSize, value);

                    int byteOffset = HEADER_SIZE + (foundRRN * 80);
                    fseek(registryBinaryFile, byteOffset, SEEK_SET);

                    free(indexArray);


                }
                //andar de registro em registro,e  comparar todos esses campos

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "proximo") == 0)
                { // if true, it means this is the filter we want! now we compare its values with each register value
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    if(temporaryRegister.proximo != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "codLinha") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    if(temporaryRegister.codLinha != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "codProxEstacao") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    if(temporaryRegister.codProxEstacao != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "distProxEstacao") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    if(temporaryRegister.distProxEstacao != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "codLinhaIntegra") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    if(temporaryRegister.codLinhaIntegra != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "codEstIntegra") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                
                    if(temporaryRegister.codEstIntegra != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "nomeEstacao") == 0)
                {
                    if (temporaryRegister.tamNomeEstacao == 0)
                    {
                    if (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") != 0)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                    }
                    else
                    {
                    if (strcmp(temporaryRegister.nomeEstacao, fieldsToBeSearched[searchForEachFilter].valueOfTheField) != 0)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "nomeLinha") == 0)
                {
                    if (temporaryRegister.tamNomeLinha == 0)
                    {
                    if (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") != 0)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                    }

                else
                {
                    if (strcmp(temporaryRegister.nomeLinha, fieldsToBeSearched[searchForEachFilter].valueOfTheField) != 0)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                }

                if(searchSucess == SEARCH_SUCESS)
                {
                    printRegistry(&temporaryRegister);
                    registersThatFulfillTheSearch++;
                }


        }

        return registersThatFulfillTheSearch;

        /* After gathering the fields we must filter, we start this specific search*/
    }

    /*if codEstacao, search using the INDEX*/
    /*if not, search using the registryBinaryFile*/
}
}

void removeIndexArchive ( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches )
{
    ;
}

void insertNewIndexArchive ( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfRegistries )
{
    ;
}

