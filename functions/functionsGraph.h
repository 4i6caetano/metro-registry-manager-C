#ifndef FUNCTIONSGRAPH_H
#define FUNCTIONSGRAPH_H

#include <stdio.h>

#define CREATEGRAPH_SUCESS  1
#define CREATEGRAPH_FAILURE 0

/**
 * @brief [10] Builds a directed weighted graph from the binary file and prints
 * the adjacency list to stdout.
 */
int createGraphFromMetro(FILE *binaryMetroFile);

/**
 * @brief [11] Finds and prints the shortest path between two stations (Dijkstra).
 * @param originField   field name for origin  (e.g. "nomeEstacaoOrigem")
 * @param originValue   station name for origin
 * @param destField     field name for destination
 * @param destValue     station name for destination
 */
int calculateLowestDistanceFromStations(FILE *binaryMetroFile,
                                        const char *originField,
                                        const char *originValue,
                                        const char *destField,
                                        const char *destValue);

/**
 * @brief [12] Builds the MST (Prim) of the undirected version of the graph and
 * prints a DFS traversal of the MST starting from originValue.
 */
int optimizeRailingSystem(FILE *binaryMetroFile,
                          const char *nameStation,
                          const char *originValue);

/**
 * @brief [13] Counts simple cycles reachable from originValue and prints the result.
 */
int countCyclesBetweenStations(FILE *binaryMetroFile,
                               const char *nameStation,
                               const char *stationValue);

#endif
