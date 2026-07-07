#ifndef GRAPHUTILS_H
#define GRAPHUTILS_H

#include <stdio.h>
#include "graph.h"
#include "registry.h"

/** Creates a Vertex with a copy of the station name. */
Vertex createVertex(Registry *reg);

/**
 * Creates a Connection for destination station destName / distancia dist /
 * line name lineName.  All three are plain C-strings (not registry pointers).
 */
Connection *createConnection(const char *destName, int dist, const char *lineName);

/**
 * Inserts connection into vertex's adjacency list keeping ascending order by
 * connectionName.  On a name tie, keeps ascending order by the first line name.
 */
void insertConnectionOnVertex(Vertex *vertex, Connection *connection);

/**
 * Adds lineName to connection->lines and keeps the array sorted ascending.
 * Does NOT add duplicates.
 */
void addLineToConnection(Connection *connection, const char *lineName);

/** qsort comparator: ascending by stationName. */
int compareVertexNameForQsort(const void *a, const void *b);

/** Binary search on sorted listOfVertices; returns pointer or NULL. */
Vertex *binarySearchOnGraph(AdjacencyList *list, int n, const char *name);

/** Prints the full adjacency list to stdout in the format required by [10]. */
void printAdjacencyList(AdjacencyList *list);

/** Frees all heap memory owned by the adjacency list. */
void freeAdjacencyList(AdjacencyList *list);

#endif
