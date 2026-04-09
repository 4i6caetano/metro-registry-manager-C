#ifndef FUNCS_H
#define FUNCS_H

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "registry.h"
#include "header.h"

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
int csvToBinary(FILE* inputCSVFile);

int showData(FILE* inputFileBinary);
/*-> Receber um binario, converter ele em string, e printar cada registro como uma linha, todo nulo deve ser NULO. 
Caso não tenha registrtos -> Registro inexistente
Erro encontrado -> Falha no processamento do arquivo
*/


void searchData(FILE* binaryFile);

int restoreRegistryBinary(FILE* binaryFile, int RRN);


#endif

