#ifndef FUNCS_H
#define FUNCS_H

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "registry.h"

int writeCSVtoBinary(FILE* inputFileCSV, FILE* outputFileBinary);


int readRegistryBinary(FILE* inputFileBinary);
#endif

