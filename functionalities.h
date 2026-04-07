#ifndef FUNCS_H
#define FUNCS_H

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "registry.h"

/**
 * @brief Logic: csv -> record in binary file, then show binary output
 * 
 * * Search:
 * - convert csv file to binary
 * - generation of binary file
 * *
 * @param inputCSVFILE .csv file that contains values of the registry's fields. 
 * * @param outputBinaryFile .bin generated using the .csv file, written by binarioNaTela
 */
int writeCSVtoBinary(FILE* inputCSVFile, FILE* outputBinaryFile);


int selectRegistryBinary(FILE* inputFileBinary);

int whereRegistryBinary(FILE* inputFileBinary, int n, char* nomeCampo, char* valorCampo);

int restoreRegistryBinary(FILE* inputfileBinary, int RRN);


#endif

