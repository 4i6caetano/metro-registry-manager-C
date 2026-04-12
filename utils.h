#ifndef UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "header.h"
#include "registry.h"

#define BINARY_TO_REGISTRY_SUCESS 1
#define BINARY_TO_REGISTRY_FAILURE 0

void BinarioNaTela(char *arquivo);
void ScanQuoteString(char *str);

char* getToken(char** buffer);
void fillRegistry(char* buffer, Registry *newRegistry);
void registryToBinary(Registry *newRegistry, FILE* outputBinaryFile);
int binaryToRegistry(Registry* newRegistry, FILE* binaryFile);
void printRegistry(Registry* newRegistry);
void freeRegistry(Registry* newRegistry);



#endif