#ifndef GRAPHUTILS_H
#define GRAPHUTILS_H

#include<stdio.h>

#include "graph.h"
#include "header.h"
#include "registry.h"

AdjacencyList createAdjacencyList(FILE* binaryMetroFile);

Vertex createVertex(Registry* registryFromBinaryFile);

Connection* createConnection(Registry* registryFromBinaryFile);

void insertConnectionOnVertex(Vertex* vertex, Connection* connection);

int compareVertexNameForQsort(const void *a, const void *b);

void addLineToConnection(Connection* connection, char* newLineName);

Vertex* binarySearchOnGraph(AdjacencyList* adjacencyList, int numberOfVertices, char *nameToBeSearched);

#endif