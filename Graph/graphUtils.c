#include<stdio.h>

#include "header.h"

#include "graph.h"
#include "graphUtils.h"
#include "registry.h"

AdjacencyList createAdjacencyList(FILE* binaryMetroFile)
{
    // pegar o numero de estacoes unicas que existem no arquivo.
    // isso esta no header. temos que ler 

    Header header;
    readHeader(binaryMetroFile, &header);

    Vertex firstVertex;

    AdjacencyList adjacencyList;

    int numberOfUniqueStations = header.nroEstacoes;

    // this way, we initiate the structure of our graph.
    adjacencyList.listOfVertices = (Vertex*) malloc(sizeof(Vertex) * numberOfUniqueStations);
    adjacencyList.numberOfVertices = numberOfUniqueStations;

    return adjacencyList;
}

Vertex createVertex(Registry *registryFromBinaryFile)
{
    Vertex vertex;
    int nameSize = registryFromBinaryFile->tamNomeEstacao;

    vertex.stationName = (char*) malloc(sizeof(char) * nameSize + 1);
    strcpy(vertex.stationName, registryFromBinaryFile->nomeEstacao);

    return vertex;
}