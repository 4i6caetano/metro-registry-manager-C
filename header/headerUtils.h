#ifndef HEADERUTILS_H
#define HEADERUTILS_H

#include <stdio.h>

/**
 * @brief Writes the initial default header to a newly created binary data file.
 *
 * @details This function is called when a new binary file is being created. It seeks 
 * to the absolute beginning of the file (byte 0) and writes the initial header values. 
 * The file is marked as inconsistent (status = '0') to indicate that data writing 
 * is in progress. The top of the removed records stack (`topo`) is initialized to -1, 
 * and the next available RRN (`proxRRN`), unique station count (`nroEstacoes`), and 
 * unique pair count (`nroParesEstacao`) are all initialized to 0.
 *
 * @param binaryFile Pointer to the binary data file (must be open for writing).
 */
void writeInitialHeader(FILE *binaryFile);

/**
 * @brief Updates the header fields after all records have been fully processed.
 *
 * @details This function should be called right before safely closing the file 
 * after a batch of insertions or after converting a CSV to binary. It seeks back 
 * to the beginning of the file, marks the status as consistent (status = '1'), 
 * skips over the `topo` byte offset (since the stack isn't modified here), and 
 * updates the `proxRRN`, `nroEstacoes`, and `nroParesEstacao` fields with the 
 * final computed values.
 *
 * @param binaryFile Pointer to the binary data file (must be open for writing).
 * @param totalRegistry The total number of records written (used to define the next available RRN).
 * @param nroEstacoes The total number of unique stations counted during processing.
 * @param nroPares The total number of unique station pairs counted during processing.
 */
void updateFinalHeader(FILE *binaryFile, int totalRegistry, int nroEstacoes, int nroPares);

/**
 * @brief Adds a station name to an array of strings if it is not already present.
 *
 * @details Iterates through the existing `uniqueStations` array to check for duplicates 
 * using `strcmp`. If the `stationName` is already in the array, the function simply returns. 
 * If it is entirely new, it duplicates the string into memory using `strdup`, assigns 
 * it to the array, and increments the global count of unique stations. This is heavily 
 * used to calculate the header's `nroEstacoes` field.
 *
 * @param uniqueStations A dynamically allocated array of string pointers storing unique station names.
 * @param count Pointer to the current total count of unique stations (incremented if added).
 * @param stationName The string containing the station name to be evaluated.
 */
void addUniqueStation(char **uniqueStations, int *count, char *stationName);

/**
 * @brief Adds a pair of connected station codes to an array if the pair is unique.
 *
 * @details First, it validates the input: if either `codEstacao` or `codProxEstacao` is 
 * -1 (NULL/invalid), it ignores the pair and returns. If valid, it iterates through the 
 * `uniquePairs` array to check if this exact combination already exists. If the pair is 
 * not found, it is appended to the array and the global pair count is incremented. 
 * This is heavily used to calculate the header's `nroParesEstacao` field.
 *
 * @param uniquePairs An array of `Pair` structures tracking connected station codes.
 * @param count Pointer to the current total count of unique pairs (incremented if added).
 * @param codEstacao The primary station code (integer).
 * @param codProxEstacao The adjacent/connected station code (integer).
 */
void addUniquePair(Pair *uniquePairs, int *count, int codEstacao, int codProxEstacao);

#endif