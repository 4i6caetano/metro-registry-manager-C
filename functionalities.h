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
 * @brief This function orchestrates the conversion of a .csv text file into a custom binary format. It begins by writing an initial *inconsistent header to protect the file during the process. As it reads each line from the CSV, it populates a Registry structure, identifies *unique station names and pairs to update the header, and writes the data into the binary file. 
 * 
 * Finally, it updates the header with the total *number of records and unique counts, setting the file status to consistent.
 * 
 * @param inputCSVFILE .csv file that contains values of the registry's fields. 
 * * @param binaryFile .bin generated using the .csv file, written by binarioNaTela
 */
int csvToBinary(FILE* inputCSVFile, FILE* binaryFile);


/**
 * @brief This function performs a sequential scan of the binary file to display all valid records. It skips the 17-byte header and reads the *file record by record. For each record, it checks the removido flag. if the record is active, it prints all fields separated by spaces, *converting any null values (indicated by -1 for integers or size 0 for strings) to the literal string "NULO".
 * 
 *If no valid records are found, *it outputs "Registro inexistente.".
 * 
 * @param binaryFile given binary file with the records.
 */
int showData(FILE* binaryFile);

/**
 * @brief This function allows the user to perform multiple searches based on specific field criteria. For each of the n search requests, it *scans the entire file sequentially. It compares each record's fields against the user-provided filters, correctly handling "NULO" searches by *checking for -1 in numeric fields or empty strings in variable fields. 
 * 
 * All matching, non-removed records are printed to the standard output, *with searches separated by a newline for proper formatting.
 * 
 * @param binaryFile given binaryFile to read from.
 * @param n number of different searches the user wants.
 */
void searchData(FILE* binaryFile, int n);

/**
* @brief This function provides direct access to a specific record using its Relative Record Number (RRN). It calculates the exact byte offset *using the formula HEADER_SIZE + (RRN * REGISTRY_SIZE) and jumps directly to that position using fseek.
*
*It then reads the single record and, if *it has not been logically removed, displays its data.
 * 
 * @param binaryFile given binary file to be read from.
 * @param RRN the Relative Record Number the user wants.
 */
int searchByRRN(FILE* binaryFile, int RRN);

#endif

