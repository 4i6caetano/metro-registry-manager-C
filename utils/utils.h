#ifndef UTILS_H
#define UTILS_H

#include "registry.h"

#define SEARCH_SUCESS 1
#define SEARCH_FAILURE 0

    /**
     * @brief Structure used to store search filters or update fields dynamically.
     *
     * @details This structure holds a key-value pair representing a field name and its corresponding 
     * target value. It is heavily used in search and update functionalities to parse user input 
     * (e.g., storing "codEstacao" in nameOfTheField and "12" in valueOfTheField) and apply these 
     * rules to the records in the data file.
     */
    typedef struct field
    {
        char nameOfTheField[100];
        char valueOfTheField[100];
    } Field;

    struct index;
    typedef struct index Index;


/**
 * @brief Catches the name of the binary FILE we want to print onto the screen, and reads it until the EOF, saving it into
 * a buffer. In the end, it prints out the binary in the terminal.
 */
void BinarioNaTela(char *arquivo);

/**
 * @brief
 * Serves with the sole purpose to read user's input and handling it accordingly, using getchar() to navigate.
 * Changes directly the memory of the string it receives. Handles:
 * 1. Invalid chars.
 * 2. fields of NULO.
 * 3. Words with space.
 */
void ScanQuoteString(char *str);

/**
 * @brief This function is responsible for returning every 'token' (word) of the .csv file, delimited by commas. It reads until the first
 * comma occurence, returns the word, and then continues until EOF.
 *
 * getToken() is used as a substitute with some additions to the function strtok(), as it was made to include the reading of 'null' elements in the .csv file.
 */
char *getToken(char **buffer);

/**
 * @brief Comparator function used by qsort to order the Index type array.
 *
 * @details This function casts the generic void pointers to the Index structure type 
 * and compares their 'codEstacao' fields. It is essential for building the primary 
 * index file, ensuring that the index entries are strictly sorted in ascending order 
 * based on the primary key.
 *
 * @param a Pointer to the first Index element.
 * @param b Pointer to the second Index element.
 * @return int Returns a negative value if a < b, 0 if a == b, and a positive value if a > b.
 */
int compareCodEstacao(const void *a, const void *b);

/**
 * @brief Performs a binary search on an array of index entries in memory.
 *
 * @details This function efficiently searches for a specific primary key (`codEstacao`) 
 * within a pre-loaded, sorted array of `Index` structures. Since the array is strictly 
 * ordered by `codEstacao`, the binary search operates in O(log N) time complexity.
 *
 * @param indexArray The dynamically allocated array of Index structures loaded in RAM.
 * @param size The total number of elements in the index array.
 * @param targetKey The primary key (`codEstacao`) to search for.
 * @return int Returns the corresponding Relative Record Number (RRN) if the key is found, 
 * or -1 if the key does not exist in the index.
 */
int binarySearchOnIndex(Index *indexArray, int size, int targetKey);

/**
 * @brief Performs a sequential scan through the data file applying multiple search filters.
 *
 * @details This function iterates over the binary data file starting from its current 
 * file pointer position. For every valid (non-removed) record, it evaluates all provided 
 * filters (`fieldsToBeSearched`). If a record matches every single filter, it is printed 
 * to the standard output, and the `registersThatFulfillTheSearch` counter is incremented. 
 * The function stops when it reaches the End of File (EOF).
 *
 * @param temporaryRegister A temporary Registry structure used as a memory buffer.
 * @param registryBinaryFile Pointer to the binary data file (must be open for reading).
 * @param numberOfFiltersApplied The number of filters/conditions to check.
 * @param fieldsToBeSearched Array of Field structures containing the keys and values to match.
 * @param registersThatFulfillTheSearch Pointer to a counter tracking how many records matched.
 */
void sequentialSearchInRegister(Registry temporaryRegister, FILE *registryBinaryFile, int numberOfFiltersApplied, Field *fieldsToBeSearched, int *registersThatFulfillTheSearch);


/**
 * @brief Evaluates a single record at the current file pointer against search filters.
 *
 * @details This function reads exactly one record from the binary data file. It is typically 
 * used immediately after an indexed search `fseek` jumps to a specific offset. It checks 
 * if the record is valid and if it satisfies any additional search filters provided by the 
 * user. If all conditions are met, the record is printed, and the match counter is incremented.
 *
 * @param temporaryRegister A temporary Registry structure used as a memory buffer.
 * @param registryBinaryFile Pointer to the binary data file (must be positioned at the target RRN).
 * @param fieldsToBeSearched Array of Field structures containing the keys and values to match.
 * @param numberOfFiltersApplied The number of filters/conditions to check.
 * @param registersThatFulfillTheSearch Pointer to a counter tracking how many records matched.
 */
void singleSearchInRegister(Registry temporaryRegister, FILE *registryBinaryFile, Field *fieldsToBeSearched, int numberOfFiltersApplied, int *registersThatFulfillTheSearch);

#endif