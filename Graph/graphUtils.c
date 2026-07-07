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

Connection* createConnection(Registry* registryFromBinaryFile)
{
    Connection* connection = (Connection*) malloc(sizeof(Connection));
    connection->numberOfLines = 0;

    connection->connectionName = (char*) malloc(sizeof(char) * registryFromBinaryFile->tamNomeEstacao + 1);
    strcpy(connection->connectionName, registryFromBinaryFile->nomeEstacao);

    connection->distanceOfNextStation = registryFromBinaryFile->distProxEstacao;

    connection->lines = (char**) malloc(sizeof(char*));
    connection->lines[0] = (char*) malloc(sizeof(char) * registryFromBinaryFile ->tamNomeLinha + 1);
    strcpy(connection->lines[0], registryFromBinaryFile->nomeLinha);
    connection->numberOfLines++;

    connection->nextVertex = NULL;
    connection->nextConnection = NULL;

    return connection;
}

int compareVertexNameForQsort(const void *a, const void *b)
{
    const Vertex *valueA = (Vertex *)a;
    const Vertex *valueB = (Vertex *)b;

    return (strcmp(valueA->stationName, valueB->stationName));
}

void addLineToConnection(Connection* connection, char* newLineName)
{
    connection->numberOfLines++;

    connection->lines = (char**) realloc(connection->lines, sizeof(char*) * connection->numberOfLines);

    connection->lines[connection->numberOfLines - 1] = (char*) malloc(strlen(newLineName) + 1);
    strcpy(connection->lines[connection->numberOfLines - 1], newLineName);
}

Vertex* binarySearchOnGraph(AdjacencyList* adjacencyList, int numberOfVertices, char *nameToBeSearched)
{
    int start = 0;
    int end = numberOfVertices - 1;

    while (start <= end)
    {
        int middle = start + (end - start) / 2;

        // Se encontramos o código da estação no índice
        if (strcmp(adjacencyList->listOfVertices[middle].stationName, nameToBeSearched) == 0) //equal
        {
            return &adjacencyList->listOfVertices[middle]; // retornar o vertice
        }

        // Se o código procurado for maior, descarta a metade esquerda
        if (strcmp(adjacencyList->listOfVertices[middle].stationName, nameToBeSearched) < 0)
        {
            start = middle + 1;
        }
        // Se o código procurado for menor, descarta a metade direita
        else
        {
            end = middle - 1;
        }
    }

    return NULL;
}


/* Criar a conexao */
/* Inserir no vértice */
/* Inserir se ja tiver conexão */