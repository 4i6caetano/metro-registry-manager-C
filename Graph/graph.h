#ifndef GRAPH_H
#define GRAPH_H

typedef struct vertex     Vertex;
typedef struct connection Connection;

/**
 * @brief Represents a directed weighted edge in the adjacency list.
 *
 * Each Connection links a source vertex to one destination station,
 * storing all line names that share this edge and the distance.
 */
typedef struct connection
{
    char       *connectionName;        /**< name of the destination station */
    int         distanceOfNextStation; /**< distance in metres (0 for integration edges) */
    char      **lines;                 /**< array of line names for this edge, kept sorted */
    int         numberOfLines;         /**< number of line names currently stored */

    Connection *nextConnection;        /**< next edge in this vertex's adjacency list */
} Connection;

/**
 * @brief Represents a station (vertex) in the directed graph.
 */
typedef struct vertex
{
    char       *stationName; /**< unique station name used as vertex label */
    Connection *nextStation; /**< head of this vertex's sorted adjacency list */
} Vertex;

/**
 * @brief Adjacency-list representation of the metro directed weighted graph.
 *
 * listOfVertices is a dynamically allocated array sorted in ascending order
 * by stationName.  Each element points to a linked list of Connection nodes
 * sorted in ascending order by connectionName (and by line name on ties).
 */
typedef struct adjacencyList
{
    Vertex *listOfVertices;   /**< sorted vertex array */
    int     numberOfVertices; /**< number of vertices */
} AdjacencyList;

#include "graphUtils.h"

#endif
