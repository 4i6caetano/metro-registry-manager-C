#ifndef HEADERUTILS_H
#define HEADERUTILS_H

#include <stdio.h>

/** @brief This project has default values for initialized headers, hence, the function writes all of them into the struct variables using fwrite().*/
void writeInitialHeader(FILE *binaryFile);

/**
 * @brief After a finished process, this function changes its header to its updated values. Called when ending a function.
 */
void updateFinalHeader(FILE *binaryFile, int totalRegistry, int nroEstacoes, int nroPares);

/**
 * @brief this function controls the unique metro station names for the file header. It iterates through the uniqueStations arrays and checks if it has already been recorded. If its new, adds it to the list.
 */
void addUniqueStation(char **uniqueStations, int *count, char *stationName);

/**
 * This functions control unique travel segments between two specific stations. Check if one of them is NULL (if positive, ignored.). If it exists and no match is found, its added.
 */
void addUniquePair(Pair *uniquePairs, int *count, int codEstacao, int codProxEstacao);

#endif