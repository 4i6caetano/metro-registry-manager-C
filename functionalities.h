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
 * @brief csvTobinary orchestrates the conversion of a .csv given file into a custom binary format, using the Registry type as it mould.
 * 
 * It begins writing an initial incosistent header, because the file is being changed.
 * 
 * As it reads each line, it populares a Registry type structure, using the fixed type sizes, and write them in the binary file.
 * 
 * Then, the records and header is updated.
 * @param inputCSVFILE .csv file that contains values of the registry's fields. 
 * * @param binaryFile .bin generated using the .csv file, written by binarioNaTela
 */
int csvToBinary(FILE* inputCSVFile, FILE* binaryFile);


/**
 * @brief showData() is responsible for scanning sequentially a given binary file, and display all its valid records.
 * 
 * For each record, it checks if the record is active. If positive, it prints it fields separated by spaces, converting any null values to 'NULL'.
 * 
 * If nothing is found, prints "Registro inexistente."
 * 
 * @param binaryFile given binary file with the records.
 */
int showData(FILE* binaryFile);

/**
 * @brief searchData() allow the user to perform multiple searches using different filters. For each n search requests, it scans the entire file sequentially.
 * 
 * If there's no values, "NULO" is used. All matching and non-removed records are printed.
 * 
 * @param binaryFile given binaryFile to read from.
 * @param n number of different searches the user wants.
 */
void searchData(FILE* binaryFile, int n);

/**
 * This function provides direct acess to a specific record using the Relative REcord Number, by the formula HEADER_SIZE + (RRN * REGISTRY_SIZE), and jumps to that position using fseek.
 * 
 * If not logically removed, display data.
 * 
 * @param binaryFile given binary file to be read from.
 * @param RRN the Relative Record Number the user wants.
 */
int searchByRRN(FILE* binaryFile, int RRN);

#endif

