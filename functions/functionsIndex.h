#ifndef FUNCTIONSINDEX_H
#define FUNCTIONSINDEX_H

#define SEARCH_SUCESS 1
#define SEARCH_FAILURE 0

#include <stdio.h>

void createPrimaryIndexArchiveInBinary( FILE *registryBinaryFile, FILE *primaryIndexArchive );

int searchOnIndexArchive( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches );

void removeIndexArchive ( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches );

void insertNewIndexArchive ( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfRegistries );

#endif