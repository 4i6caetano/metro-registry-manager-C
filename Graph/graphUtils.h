#ifndef GRAPHUTILS_H
#define GRAPHUTILS_H

#include<stdio.h>

#include "graph.h"
#include "header.h"
#include "registry.h"

AdjacencyList createAdjacencyList(FILE* binaryMetroFile);

Vertex createVertex(Registry* registryFromBinaryFile);

#endif