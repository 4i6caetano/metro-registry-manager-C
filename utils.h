#ifndef UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "header.h"
#include "registry.h"
#include "functionalities.h"

#define BINARY_TO_REGISTRY_SUCESS 1
#define BINARY_TO_REGISTRY_FAILURE 0

void BinarioNaTela(char *arquivo);
void ScanQuoteString(char *str);

/**
 * @brief getToken() is used as a substitute with some additions to the function strtok(), as it was made to include the reading of 'null' elements in the .csv file.
 * 
 * This function pick up the data from the .csv file and save it into a buffer.
 */
char* getToken(char** buffer);

#endif