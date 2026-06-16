#ifndef FUNCTIONSINDEX_H
#define FUNCTIONSINDEX_H

#include <stdio.h>


/**
 * @brief Creates a primary index file based on an existing data binary file.
 *
 * @details This function reads through the entire data file, ignoring any logically 
 * removed records. For every valid record found, it extracts the primary key 
 * (`codEstacao`) and its corresponding Relative Record Number (RRN). It stores these 
 * key-RRN pairs in an array in memory, sorts the array in ascending order using 
 * the primary key, and then writes the ordered pairs into the primary index file. 
 * Finally, it updates the status of the index file to mark it as consistent.
 *
 * @param registryBinaryFile Pointer to the input data binary file (must be open for reading).
 * @param primaryIndexArchive Pointer to the output primary index binary file (must be open for writing).
 * @return int Returns FUNCTION_SUCESS if the operation completes successfully, 
 * or FUNCTION_FAILURE if there is an error with the file pointers.
 */
int createPrimaryIndexArchiveInBinary( FILE *registryBinaryFile, FILE *primaryIndexArchive );


/**
 * @brief Searches for records in the data file based on multiple user-defined criteria.
 *
 * @details Iterates `numberOfSearches` times. For each iteration, it reads the desired 
 * filters from standard input. If the primary key (`codEstacao`) is among the filters, 
 * it optimizes the process by executing an indexed search: it loads the index file into RAM, 
 * performs a binary search to find the RRN, and directly seeks that byte offset in the 
 * data file. If the primary key is not provided, it falls back to a sequential search, 
 * reading every record from the beginning of the data file. It prints the records that 
 * match all specified criteria.
 *
 * @param registryBinaryFile Pointer to the data binary file (must be open for reading).
 * @param primaryIndexArchive Pointer to the primary index binary file (must be open for reading).
 * @param numberOfSearches The number of independent search operations to execute.
 * @return int Returns FUNCTION_SUCESS upon completion, or FUNCTION_FAILURE if files are invalid.
 */
int searchOnIndexArchive( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches );

/**
 * @brief Logically removes records matching specified criteria and updates the index file.
 *
 * @details Executes `numberOfSearches` removal operations. For each operation, it locates 
 * the matching records (using binary search on the index if `codEstacao` is given, or 
 * sequential search otherwise). When a match is found, the record is logically removed 
 * by setting its `removido` byte to '1'. It then pushes the removed record's RRN onto 
 * the logical removal stack (updating the `topo` in the header and the `proximo` field 
 * in the record). Simultaneously, it removes the corresponding entry from the primary 
 * index file to ensure consistency.
 *
 * @param registryBinaryFile Pointer to the data binary file (must be open for reading and writing).
 * @param primaryIndexArchive Pointer to the primary index binary file (must be open for reading and writing).
 * @param numberOfSearches The number of independent removal operations to execute.
 * @return int Returns FUNCTION_SUCESS upon successful removals, or FUNCTION_FAILURE on error.
 */
int removeIndexArchive ( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches );

/**
 * @brief Inserts new records into the data file, reusing space from logically removed records.
 *
 * @details Reads `numberOfRegistries` new records from standard input. For each new record, 
 * it checks the logical removal stack (indicated by the `topo` field in the header). If the 
 * stack is not empty, it pops the top RRN and reuses that byte offset, writing the new 
 * record into the freed space. If the stack is empty, it appends the new record to the end 
 * of the file and increments `proxRRN`. After physically writing the record, it inserts 
 * the new `codEstacao` and its RRN into the primary index file, shifting elements as 
 * needed to maintain ascending order.
 *
 * @param registryBinaryFile Pointer to the data binary file (must be open for reading and writing).
 * @param primaryIndexArchive Pointer to the primary index binary file (must be open for reading and writing).
 * @param numberOfRegistries The number of new records to insert.
 * @return int Returns FUNCTION_SUCESS upon successful insertions, or FUNCTION_FAILURE on error.
 */
int insertNewIndexArchive ( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfRegistries );

/**
 * @brief Performs in-place updates on records that match specific search criteria.
 *
 * @details Executes `numberOfUpdates` operations. First, it reads the search filters to 
 * locate the target records (using indexed binary search or sequential search). Then, it 
 * reads the fields that need to be updated. Once a matching record is found, the specified 
 * fields are modified directly at its current byte offset in the binary file (in-place update, 
 * avoiding full record rewrites if possible). If the update modifies the primary key 
 * (`codEstacao`), the function also deletes the old key from the primary index and inserts 
 * the new key with the same RRN to maintain the integrity of the index.
 *
 * @param registryBinaryFile Pointer to the data binary file (must be open for reading and writing).
 * @param primaryIndexArchive Pointer to the primary index binary file (must be open for reading and writing).
 * @param numberOfUpdates The number of update operations to execute.
 * @return int Returns FUNCTION_SUCESS upon successful updates, or FUNCTION_FAILURE on error.
 */
int updateIndexArchive( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfUpdates );

#endif
