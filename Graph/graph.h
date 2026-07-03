#ifndef GRAPH_H
#define GRAPH_H

typedef struct vertex Vertex;
typedef struct connection Connection;

typedef struct vertex
{
    char* stationName;
    Connection* nextStation;

} Vertex;

/**
 *   char* nameOfNextStation;
 *   int distanceOfNextStation;
 *   char** lines;
 *
 *  Vertex* nextVertex;
 *  Connection* nextConnection;
 */
typedef struct connection
{
    char* nameOfNextStation;

    int distanceOfNextStation;

    char** lines;
    int numberOfLines;

    Vertex* nextVertex;
    Connection* nextConnection;

} Connection;

typedef struct adjacencyList
{

    Vertex* listOfVertices;
    int numberOfVertices;

} AdjacencyList;

#include "graphUtils.h"

#endif