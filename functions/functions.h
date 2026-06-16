#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define FUNCTION_SUCESS 1
#define FUNCTION_FAILURE 0

#include <stdio.h>

// Functions implemented on the FIRST issue of the project

/**
 * @brief Orchestrates the conversion of a raw CSV text file into a structured binary format.
 *
 * @details This routine begins by writing an initial inconsistent header to protect the file 
 * during the conversion process. It sequentially reads each line from the CSV input, parses 
 * the text fields, and serializes the data into fixed-size binary records. During this process, 
 * it actively tracks unique station names and connected station pairs. Finally, it rewrites 
 * the header with the total number of records and unique counts, officially marking the file 
 * status as consistent.
 *
 * @param inputCSVFile Pointer to the source CSV text file (must be open for reading).
 * @param binaryFile Pointer to the destination binary file (must be open for writing).
 * @return int Returns a success macro if the conversion completes successfully, or a failure macro on error.
 */
int csvToBinary(FILE *inputCSVFile, FILE *binaryFile);

/**
 * @brief Performs a sequential scan of the binary file to display all valid records.
 *
 * @details This operation skips the initial header and reads the binary file record by record. 
 * For each record, it evaluates the logical removal flag. If the record is active, its fields 
 * are printed to the terminal separated by spaces, with any null values (indicated by -1 for 
 * integers or size 0 for strings) replaced by the literal string "NULO". If no valid records 
 * exist in the entire file, it outputs a standard "not found" message.
 *
 * @param binaryFile Pointer to the binary data file (must be open for reading).
 * @return int Returns a success macro upon completion, or a failure macro if the file pointer is invalid.
 */
int displayValidRecords(FILE *binaryFile);

/**
 * @brief Linearly scans the binary file to execute multiple independent search queries.
 *
 * @details This procedure executes a specified number of search blocks. For each query, it reads 
 * multiple column-value filtering pairs and sequentially evaluates every active record in the file 
 * against these criteria. Fixed fields are evaluated via integer matching, while variable-length 
 * fields utilize string comparisons. It strictly manages memory by freeing dynamic heap allocations 
 * after evaluating each record to prevent memory leaks. If a query yields no results, it prints 
 * a standard "not found" message.
 *
 * @param binaryFile Pointer to the binary data file (must be open for reading).
 * @param n The number of independent search operations to perform.
 */
void searchData(FILE *binaryFile, int n);

/**
 * @brief Provides direct access to a specific record using its Relative Record Number (RRN).
 *
 * @details This operation calculates the exact byte offset of the target record using a fixed 
 * mathematical formula based on the header size and the standard record size. It jumps directly 
 * to that position in the file using random access and reads the data block. If the record exists 
 * and has not been logically removed, its data is formatted and printed to the terminal. Otherwise, 
 * it outputs a standard "not found" message.
 *
 * @param binaryFile Pointer to the binary data file (must be open for reading).
 * @param RRN The target Relative Record Number to retrieve.
 * @return int Returns a success macro if the record is found and valid, or a failure macro if it is removed/invalid.
 */
int searchByRRN(FILE *binaryFile, int RRN);

// Functions implemented on the SECOND issue of the project.

#endif
