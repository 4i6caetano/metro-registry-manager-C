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

void restoreIndexArchive( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches)
{
    ;
}

void removeIndexArchive ( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches )
{
    ;
}

void insertNewIndexArchive ( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfRegistries )
{
    ;
}

