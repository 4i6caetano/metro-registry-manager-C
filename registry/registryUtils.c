#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "registry.h"
#include "utils.h"


void fillRegistry(char *buffer, Registry *newRegistry)
{
    buffer[strcspn(buffer, "\r\n")] = '\0';

    char *rest = buffer;
    char *token;

    token = getToken(&rest);
    newRegistry->codEstacao = atoi(token);

    token = getToken(&rest);
    newRegistry->tamNomeEstacao = strlen(token);
    newRegistry->nomeEstacao = strdup(token);

    token = getToken(&rest);
    newRegistry->codLinha = (strlen(token) == 0) ? -1 : atoi(token);

    token = getToken(&rest);
    newRegistry->tamNomeLinha = strlen(token);
    newRegistry->nomeLinha = ((newRegistry->tamNomeLinha) == 0) ? NULL : strdup(token);

    token = getToken(&rest);
    newRegistry->codProxEstacao = (strlen(token) == 0) ? -1 : atoi(token);

    token = getToken(&rest);
    newRegistry->distProxEstacao = (strlen(token) == 0) ? -1 : atoi(token);

    token = getToken(&rest);
    newRegistry->codLinhaIntegra = (strlen(token) == 0) ? -1 : atoi(token);

    token = getToken(&rest);
    newRegistry->codEstIntegra = (strlen(token) == 0) ? -1 : atoi(token);

    newRegistry->removido = '0';
    newRegistry->proximo = -1;
}; // The csv file comes in a predetermined order. In that sense, this function reads the words and store them in the right field.

void registryToBinary(Registry *newRegistry, FILE *outputBinaryFile)
{

    /* Write the fields with fixed size */
    fwrite(&newRegistry->removido, sizeof(char), 1, outputBinaryFile);
    fwrite(&newRegistry->proximo, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->codEstacao, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->codLinha, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->codProxEstacao, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->distProxEstacao, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->codLinhaIntegra, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->codEstIntegra, sizeof(int), 1, outputBinaryFile);


    fwrite(&newRegistry->tamNomeEstacao, sizeof(int), 1, outputBinaryFile);

    if (newRegistry->tamNomeEstacao > 0)
    {
        fwrite(newRegistry->nomeEstacao, sizeof(char), newRegistry->tamNomeEstacao, outputBinaryFile);
        free(newRegistry->nomeEstacao);
    }

    fwrite(&newRegistry->tamNomeLinha, sizeof(int), 1, outputBinaryFile);

    if (newRegistry->tamNomeLinha > 0)
    {
        fwrite(newRegistry->nomeLinha, sizeof(char), newRegistry->tamNomeLinha, outputBinaryFile);
        free(newRegistry->nomeLinha);
    }

    int byteCount = 37 + newRegistry->tamNomeEstacao + newRegistry->tamNomeLinha;
    int garbage = 80 - byteCount;
    char dolar = '$';

    for (int i = 0; i < garbage; i++)
    {
        fwrite(&dolar, sizeof(char), 1, outputBinaryFile);
    }
}

int binaryToRegistry(Registry *newRegistry, FILE *binaryFile)
{

    if (binaryFile == NULL) //error ha ndling
        return BINARY_TO_REGISTRY_FAILURE;

    if (fread(&newRegistry->removido, sizeof(char), 1, binaryFile) != 1)
    {
        return BINARY_TO_REGISTRY_FAILURE; //if removed, returns failure.
    }

    /* reads the data from the binaryFile and saves them on the registry. */

    fread(&newRegistry->proximo, sizeof(int), 1, binaryFile);
    fread(&newRegistry->codEstacao, sizeof(int), 1, binaryFile);
    fread(&newRegistry->codLinha, sizeof(int), 1, binaryFile);
    fread(&newRegistry->codProxEstacao, sizeof(int), 1, binaryFile);
    fread(&newRegistry->distProxEstacao, sizeof(int), 1, binaryFile);
    fread(&newRegistry->codLinhaIntegra, sizeof(int), 1, binaryFile);
    fread(&newRegistry->codEstIntegra, sizeof(int), 1, binaryFile);

    fread(&newRegistry->tamNomeEstacao, sizeof(int), 1, binaryFile);

    if (newRegistry->tamNomeEstacao > 0)
    {
        newRegistry->nomeEstacao = (char *)malloc(newRegistry->tamNomeEstacao + 1);
        fread(newRegistry->nomeEstacao, sizeof(char), newRegistry->tamNomeEstacao, binaryFile);
        newRegistry->nomeEstacao[newRegistry->tamNomeEstacao] = '\0';
    }
    else
    {
        newRegistry->nomeEstacao = NULL;
    }

    fread(&newRegistry->tamNomeLinha, sizeof(int), 1, binaryFile);

    if (newRegistry->tamNomeLinha > 0)
    {
        newRegistry->nomeLinha = (char *)malloc(newRegistry->tamNomeLinha + 1);
        fread(newRegistry->nomeLinha, sizeof(char), newRegistry->tamNomeLinha, binaryFile);
        newRegistry->nomeLinha[newRegistry->tamNomeLinha] = '\0';
    }
    else
    {
        newRegistry->nomeLinha = NULL;
    }

    int countBytes = 37 + newRegistry->tamNomeEstacao + newRegistry->tamNomeLinha;
    int memoryGarbage = 80 - countBytes;

    if (memoryGarbage > 0)
    {
        fseek(binaryFile, memoryGarbage, SEEK_CUR);
    }

    return BINARY_TO_REGISTRY_SUCESS;
}

void printRegistry(Registry *newRegistry)
{

    if (newRegistry->codEstacao == -1)
        printf("NULO ");
    else
        printf("%d ", newRegistry->codEstacao);

    if (newRegistry->tamNomeEstacao == 0)
        printf("NULO ");
    else
        printf("%s ", newRegistry->nomeEstacao);

    if (newRegistry->codLinha == -1)
        printf("NULO ");
    else
        printf("%d ", newRegistry->codLinha);

    if (newRegistry->tamNomeLinha == 0)
        printf("NULO ");
    else
        printf("%s ", newRegistry->nomeLinha);

    if (newRegistry->codProxEstacao == -1)
        printf("NULO ");
    else
        printf("%d ", newRegistry->codProxEstacao);

    if (newRegistry->distProxEstacao == -1)
        printf("NULO ");
    else
        printf("%d ", newRegistry->distProxEstacao);

    if (newRegistry->codLinhaIntegra == -1)
        printf("NULO ");
    else
        printf("%d ", newRegistry->codLinhaIntegra);

    if (newRegistry->codEstIntegra == -1)
        printf("NULO \n");
    else
        printf("%d \n", newRegistry->codEstIntegra);
}

void freeRegistry(Registry *newRegistry)
{
    if (newRegistry->tamNomeEstacao > 0)
    {
        free(newRegistry->nomeEstacao);
    }
    if (newRegistry->tamNomeLinha > 0)
    {
        free(newRegistry->nomeLinha);
    }
    newRegistry->nomeEstacao = NULL;
    newRegistry->nomeLinha = NULL;
}