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

/** @brief As the file is changed anr/or initiated, the header has default type of values, like marking the status as inconsistent, that there's no next RRN yet. This function does this. */
void writeInitialHeader(FILE* binaryFile);

/**
 * @brief after finishing a certain process in the file, this function change its header to its updated values. Called when ending a function.
 */
void updateFinalHeader(FILE* binaryFile, int totalRegistry, int nroEstacoes, int nroPares);

/**
 * @brief this function controls the unique metro station names for the file header. It iterates through the uniqueStations arrays and checks if it has already been recorded. If its new, adds it to the list.
 */
void addUniqueStation(char** uniqueStations, int* count, char* stationName);

/**
 * This functions control unique travel segments between two specific stations. Check if one of them is NULL (if positive, ignored.). If it exists and no match is found, its added.
 */
void addUniquePair(Pair* uniquePairs, int* count, int codEstacao, int codProxEstacao);

#endif