#ifndef FUNCS_H
#define FUNCS_H

#define FUNCTION_SUCESS 1
#define FUNCTION_FAILURE 0

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "registry.h"
#include "header.h"
#include "utils.h"

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
int csvToBinary(FILE* inputCSVFile, FILE* binaryFile);

int showData(FILE* binaryFile);
/*-> Receber um binario, converter ele em string, e printar cada registro como uma linha, todo nulo deve ser NULO. 
Caso não tenha registrtos -> Registro inexistente
Erro encontrado -> Falha no processamento do arquivo
*/

void searchData(FILE* binaryFile, int n);

int searchByRRN(FILE* binaryFile, int RRN);

#endif

