#ifndef HEADER_H
#define HEADER_H

#define HEADER_SIZE 17
#define STATUS_CONSISTENT '1'
#define STATUS_INCONSISTENT '0'

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "header.h"
#include "registry.h"
#include "utils.h"


typedef struct header {
  char status; /**< (CHAR) indicates if the file is CONSISTENT = '0', or INCONSISTENT = '1'. By default, the value.  */
  int topo; /**< (CHAR) storages the BYTE OFFSET of the last REMOVED registry. '-1' by default. */
  int proxRRN;  /** next avaible RRN. '0' by default. */
  int nroEstacoes; /**< (INT) indicates the quantity of varied stations storaged in the data file. */
  int nroParesEstacao; /**< (INT) indicates the quantity of different PAIRS (codEstacao, codProxEstacao) storaged in the data file. */
} Header;

typedef struct pair { 
    int c1; 
    int c2; 
} Pair;

void writeInitialHeader(FILE* binaryFile);
void updateFinalHeader(FILE* binaryFile, int totalRegistry, int nroEstacoes, int nroPares);
void addUniqueStation(char** uniqueStations, int* count, char* stationName);
void addUniquePair(Pair* uniquePairs, int* count, int codEstacao, int codProxEstacao);

#endif