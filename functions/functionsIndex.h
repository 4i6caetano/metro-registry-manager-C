#ifndef FUNCTIONSINDEX_H
#define FUNCTIONSINDEX_H

#include <stdio.h>

void createPrimaryIndexArchive( FILE *registryBinaryFile, FILE *primaryIndexArchive );

void restoreIndexArchive( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches );

void removeIndexArchive ( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches );

void insertNewIndexArchive ( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfRegistries );

#endif