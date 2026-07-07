#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"

void writeInitialHeader(FILE *binaryFile)
{
    fseek(binaryFile, 0, SEEK_SET);

    char status = STATUS_INCONSISTENT;
    int topo = -1;
    int proxRRN = 0;
    int nroEstacoes = 0;
    int nroParesEstacao = 0;

    fwrite(&status, sizeof(char), 1, binaryFile);
    fwrite(&topo, sizeof(int), 1, binaryFile);
    fwrite(&proxRRN, sizeof(int), 1, binaryFile);
    fwrite(&nroEstacoes, sizeof(int), 1, binaryFile);
    fwrite(&nroParesEstacao, sizeof(int), 1, binaryFile);
}

void readHeader(FILE *arq, Header *cab)
{
    fseek(arq, 0, SEEK_SET);
    fread(&cab->status,          sizeof(char), 1, arq);
    fread(&cab->topo,            sizeof(int),  1, arq);
    fread(&cab->proxRRN,         sizeof(int),  1, arq);
    fread(&cab->nroEstacoes,     sizeof(int),  1, arq);
    fread(&cab->nroParesEstacao, sizeof(int),  1, arq);
}

void writeHeader(FILE *arq, Header *cab)
{
    fseek(arq, 0, SEEK_SET);
    fwrite(&cab->status,          sizeof(char), 1, arq);
    fwrite(&cab->topo,            sizeof(int),  1, arq);
    fwrite(&cab->proxRRN,         sizeof(int),  1, arq);
    fwrite(&cab->nroEstacoes,     sizeof(int),  1, arq);
    fwrite(&cab->nroParesEstacao, sizeof(int),  1, arq);
}

void updateFinalHeader(FILE *binaryFile, int totalRegistry, int nroEstacoes, int nroPares)
{
    char status = STATUS_CONSISTENT;

    fseek(binaryFile, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, binaryFile);

    fseek(binaryFile, sizeof(int), SEEK_CUR);

    fwrite(&totalRegistry, sizeof(int), 1, binaryFile);
    fwrite(&nroEstacoes, sizeof(int), 1, binaryFile);
    fwrite(&nroPares, sizeof(int), 1, binaryFile);
}

void addUniqueStation(char **uniqueStations, int *count, char *stationName)
{
    for (int i = 0; i < *count; i++)
    {
        if (strcmp(uniqueStations[i], stationName) == 0)
        {
            return;
        }
    }
    uniqueStations[*count] = strdup(stationName);
    (*count)++;
}

void addUniquePair(Pair *uniquePairs, int *count, int codEstacao, int codProxEstacao)
{

    if (codEstacao == -1 || codProxEstacao == -1)
        return;

    for (int i = 0; i < *count; i++)
    {
        if (uniquePairs[i].c1 == codEstacao && uniquePairs[i].c2 == codProxEstacao)
        {
            return;
        }
    }
    uniquePairs[*count].c1 = codEstacao;
    uniquePairs[*count].c2 = codProxEstacao;
    (*count)++;
}