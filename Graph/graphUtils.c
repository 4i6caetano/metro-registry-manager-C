#include<stdio.h>
#include<string.h>

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

    vertex.stationName = (char*) malloc(sizeof(char) * (nameSize + 1));

    strcpy(vertex.stationName, registryFromBinaryFile->nomeEstacao);

    vertex.nextStation = NULL;


    return vertex;
}

void insertConnectionOnVertex(Vertex* vertex, Connection* connection)
{
    if(vertex -> nextStation == NULL)
    {
        vertex->nextStation = connection;
        return;
    }

    Connection* previous = NULL;
    Connection* current = vertex->nextStation;

    while(current != NULL && strcmp(current->connectionName, connection->connectionName) < 0)
    {

        previous = current;
        current = current->nextConnection;

    }

    connection->nextConnection = current;

    if(previous ==  NULL)
    {
        vertex->nextStation = connection;
    }

    else
    {
        previous->nextConnection = connection;
    }

    //Caso não seja a primeira aresta, e sim da segunda pra frente, percorrer elas.
    
}

/*
    char* nameOfNextStation;
    int distanceOfNextStation;
    char** lines;

    Vertex* nextVertex;
    Connection* nextConnection;*/
Connection createConnection(Registry* registryFromBinaryFile)
{
    Connection connection;
    connection.numberOfLines = 0;

    connection.connectionName = (char*) malloc(sizeof(char) * registryFromBinaryFile->tamNomeEstacao + 1);
    strcpy(connection.connectionName, registryFromBinaryFile->nomeEstacao);

    connection.distanceOfNextStation = registryFromBinaryFile->distProxEstacao;

    connection.lines = (char**) malloc(sizeof(char*));
    connection.lines[0] = (char*) malloc(sizeof(char) * registryFromBinaryFile ->tamNomeLinha + 1);
    strcpy(connection.lines[0], registryFromBinaryFile->nomeLinha);
    connection.numberOfLines++;

    connection.nextVertex = NULL;
    connection.nextConnection = NULL;

    return connection;
}

int compareVertexNameForQsort(const void *a, const void *b)
{
    const Vertex *valueA = (Vertex *)a;
    const Vertex *valueB = (Vertex *)b;

    return (strcmp(valueA->stationName, valueB->stationName));
}

int binarySearchOnGraph(AdjacencyList* adjacencyList, int numberOfVertices, char *nameToBeSearched)
{
    
}


/* Criar a conexao */
/* Inserir no vértice */
/* Inserir se ja tiver conexão */