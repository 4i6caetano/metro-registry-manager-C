#ifndef UTILS_H

#include "registry.h"

#define SEARCH_SUCESS 1
#define SEARCH_FAILURE 0


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

int compareCodEstacao(const void *a, const void *b);


    /** 
    * @brief 
    * nameOfTheField[100];
    *  
    * valueOfTheField[100]; 
    * */
    typedef struct field
    {
        char nameOfTheField[100];
        char valueOfTheField[100];
    } Field;

int binarySearchOnIndex(Index *indexArray, int size, int targetKey);

void sequentialSearchInRegister(Registry temporaryRegister, FILE *registryBinaryFile, int numberOfFiltersApplied, Field *fieldsToBeSearched, int *registersThatFulfillTheSearch);

#endif