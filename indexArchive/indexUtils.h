#ifndef INDEXUTILS_H
#define INDEXUTILS_H

#include "stdio.h"

#include "registry.h"
#include "utils.h"
#include "header.h"

/**
 * @brief Scans the data file to count unique stations and pairs.
 *
 * @details This routine reads all valid (logically non-removed) records from the data 
 * file. It keeps track of station names and station code pairs that have already been 
 * seen. Ultimately, it updates the provided pointers with the total count of unique 
 * values, which is essential for maintaining header consistency after insert, remove, 
 * or update operations.
 *
 * @param arq Pointer to the binary data file (must be open for reading).
 * @param nEst Pointer to the variable that will store the number of unique stations.
 */
void countValidRecords(FILE *arq, int *nEst, int *nPares);

/**
 * @brief Calculates the total number of records present in the index file.
 *
 * @details This function moves the cursor to the end of the index file to determine 
 * its total size in bytes. It then subtracts the header size (1 byte) and divides the 
 * remainder by the fixed size of each index record (8 bytes), returning the exact 
 * amount of registered entries.
 *
 * @param arqIndice Pointer to the index file (must be open for reading).
 * @return int Returns the total number of index entries present in the file.
 */
int countNumberOfRecords(FILE *arqIndice);

/**
 * @brief Removes a specific entry from the index file on disk.
 *
 * @details This routine loads all index entries into main memory. Next, it rewrites 
 * the file from the beginning (after the header), skipping the entry that matches the 
 * specified search key. Finally, it uses system operations to truncate the file, 
 * removing any idle space or residual garbage at the end.
 *
 * @param arqIndice Pointer to the index file (must be open for reading and writing).
 * @param codEstacao The primary key of the entry to be removed.
 */
void removeByIndex(FILE *arqIndice, int codEstacao);

/**
 * @brief Inserts a new entry into the index file while maintaining order.
 *
 * @details This operation loads current entries into memory, identifies the correct 
 * insertion position to maintain the ascending order of the primary key, shifts larger 
 * elements to the right, and inserts the new value. In the end, it rewrites the 
 * updated index file to the disk.
 *
 * @param arqIndice Pointer to the index file (must be open for reading and writing).
 * @param codEstacao The primary key of the new entry to be inserted.
 * @param rrn The corresponding Relative Record Number (RRN) in the data file.
 */
void insertOnIndex(FILE *arqIndice, int codEstacao, int rrn);

/**
 * @brief Updates the fields of a record held in main memory.
 *
 * @details This routine iterates over a set of update filters and modifies the 
 * corresponding fields in the RAM data structure. It automatically handles the freeing 
 * and reallocation of memory for variable-length fields (strings) and properly handles 
 * null values, converting empty strings to -1 (for integers) or to null pointers.
 *
 * @param reg Pointer to the record structure that will be modified.
 * @param camposAtu Array containing the pairs of field names and their new values.
 * @param p The number of fields that must be updated.
 */
void updateRecords(Registry *reg, Field *camposAtu, int p);

/**
 * @brief Checks if the primary key is present among the search filters.
 *
 * @details This function scans the list of user-defined criteria to find out if the 
 * main identification key was provided. This is used to decide whether the system can 
 * optimize the operation by performing a binary search through the index.
 *
 * @param campos Array containing the provided search criteria.
 * @param m The number of criteria present in the array.
 * @return int Returns the numeric value of the primary key if it is in the filters, 
 * or -2 if the primary key is not part of the search criteria.
 */
int getCodEstacaoForSearch(Field *campos, int m);

/**
 * @brief Evaluates whether a record meets all specified search criteria.
 *
 * @details This routine iteratively compares the fields of the record loaded in memory 
 * against a list of filters. The record is only considered compatible if it positively 
 * validates against absolutely all provided criteria (a logical AND operation).
 *
 * @param reg Pointer to the record structure to be evaluated.
 * @param campos Array containing the provided search criteria.
 * @param m The number of filters to be applied.
 * @return int Returns 1 if the record matches all filters, or 0 otherwise.
 */
int isTheRegistryCorrespondent(Registry *reg, Field *campos, int m);

/**
 * @brief Reads and formats a new record from the standard input.
 *
 * @details This function captures data typed in the terminal (usually employing handling 
 * for quotes and empty strings) and populates a record structure. It handles dynamic 
 * allocation for text fields and initializes standard control fields, such as the 
 * non-removed flag and null linking pointers.
 *
 * @param reg Pointer to the record structure that will store the read data.
 */
void readRegistryStdin(Registry *reg);

/**
 * @brief Locates the RRN of a record through a binary search on the index file.
 *
 * @details Encapsulates the pattern repeated across functionalities [6], [7] and [9]:
 * loads every entry of the primary index file into RAM, performs a binary search for
 * the given primary key, frees the temporary array and returns the result. Centralizing
 * it here avoids three nearly identical copies of the same load+search+free sequence.
 *
 * @param primaryIndexArchive Pointer to the primary index file (must be open for reading).
 * @param codEstacao The primary key being searched for.
 * @return int Returns the RRN associated with codEstacao, or -1 if it is not indexed.
 */
int localizarRRNViaIndice(FILE *primaryIndexArchive, int codEstacao);

/**
 * @brief Marks both the data file and the index file as inconsistent.
 *
 * @details Used at the start of any operation that modifies disk content (remove,
 * insert, update). If the program crashes mid-operation, both files remain flagged as
 * inconsistent, signaling that they should not be trusted by later executions.
 *
 * @param registryBinaryFile Pointer to the data file (must be open for writing).
 * @param primaryIndexArchive Pointer to the primary index file (must be open for writing).
 */
void marcarArquivosInconsistentes(FILE *registryBinaryFile, FILE *primaryIndexArchive);

/**
 * @brief Marks both the data file and the index file as consistent again.
 *
 * @details Writes the updated header (with the current status, stack top, next RRN and
 * counters) back to the data file, then flags the index file as consistent. Used at the
 * end of remove/insert/update operations once every change has been safely written.
 *
 * @param registryBinaryFile Pointer to the data file (must be open for writing).
 * @param primaryIndexArchive Pointer to the primary index file (must be open for writing).
 * @param cab Pointer to the in-memory header holding the values to persist.
 */
void marcarArquivosConsistentes(FILE *registryBinaryFile, FILE *primaryIndexArchive, Header *cab);

#endif