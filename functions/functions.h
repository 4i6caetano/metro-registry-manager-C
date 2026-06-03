#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define FUNCTION_SUCESS 1
#define FUNCTION_FAILURE 0

#include <stdio.h>

// Functions implemented on the FIRST issue of the project

/**
 * @brief This function orchestrates the conversion of a .csv text file into a custom binary format. It begins by writing an initial INCONSISTENT header to protect the file during the process. As it reads each line from the CSV,
 *  using writeInitialHeader() and fillRegistry() combined with fgets(), it roams through the csv file until EOF, writing all the data onto the Registry resulting in a binary file.
 * It identifies unique station names and pairs to update the header, and writes the data into the binary file.
 *
 * Finally, it updates the header with the total *number of records and unique counts, setting the file status to consistent.
 *
 * @param inputCSVFILE .csv file that contains values of the registry's fields.
 * @param binaryFile .bin generated using the .csv file, written by binarioNaTela
 */
int csvToBinary(FILE *inputCSVFile, FILE *binaryFile);

/**
 * @brief This function performs a sequential scan of the binary using binaryToRegistry() to display all valid records into terminal. It skips the 17-byte header and reads the file record by record. 
 * For each record, it checks the removido flag. if the record is active, it prints all fields separated by spaces, converting any null values (indicated by -1 for integers or size 0 for strings) to the literal string "NULO".
 *
 * If no valid records are found, it outputs "Registro inexistente.".
 *
 * @param binaryFile given binary file with the records.
 */
int displayValidRecords(FILE *binaryFile);

/**
 * @brief 
 * Linearly scans the binary file to sequentially execute 'n' independent search blocks, reading 'm' column-value filtering pairs per query, and prints all matching records to standard output.
 * 
 * Evaluates fixed fields via integer matching (converting NULO inputs to -1) and variable fields via length-checked string comparisons (strcmp).
 * 
 * Must explicitly invoke freeRegistry() at the end of every loop iteration to safely free dynamic heap allocations and prevent catastrophic memory leaks.
 */
void searchData(FILE *binaryFile, int n);

/**
 * @brief This function provides direct access to a specific record using its Relative Record Number (RRN). It calculates the exact byte offset *using the formula HEADER_SIZE + (RRN * REGISTRY_SIZE) and jumps directly to that position using fseek.
 *
 * It then reads the single record and, if *it has not been logically removed, displays its data.
 *
 * @param binaryFile given binary file to be read from.
 * @param RRN the Relative Record Number the user wants.
 */
int searchByRRN(FILE *binaryFile, int RRN);

// Functions implemented on the SECOND issue of the project.

#endif
