#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "graph.h"
#include "functionsGraph.h"

#include "registry.h"
#include "header.h"

/*
A representação do grafo deve ser, obrigatoriamente, na forma de listas de
adjacências. As listas de adjacências consistem tradicionalmente em um vetor de |V|
elementos que são capazes de apontar, cada um, para uma lista linear, de forma que o
i-ésimo elemento do vetor aponta para a lista linear de arestas que são adjacentes ao
vértice i. Cada elemento do vetor deve representar o nome de uma estação. Um
exemplo de nome de estação é “Luz”. Os vértices do vetor devem ser ordenados de
forma crescente de acordo com o nome da estação. Note que, segundo a especificação
do trabalho prático, se duas ou mais estações têm o mesmo nome, elas são
consideradas a mesma estação.
Cada elemento da lista linear representa uma aresta entre duas estações e deve
armazenar: (i) o nome da próxima estação; (ii) a distância para a próxima estação; e
(iii) os nomes de linha associados.
*/

int createGraphFromMetro(FILE *binaryMetroFile)
{
    // ler o header e descobrir quantas estacoes unicas tem. cada estacao sera um vertice.
    // alocar a Adjacencia com esse tamanho

    // Depois, temos que ler o arquivo binário e ir colocando os vertices e arestas.
    // -> Começamos por percorrer o binário e colocar todos os vértices

    if (!binaryMetroFile)
    {
        printf("Falha no processamento do arquivo.\n");
        return CREATEGRAPH_FAILURE;
    }

    AdjacencyList adjacencyList = createAdjacencyList(binaryMetroFile); // We alocate the memory and stations needed on the list

    Vertex vertex;
    Connection *connection;
    Registry registryFromBinaryFile;

    fseek(binaryMetroFile, HEADER_SIZE, SEEK_SET);
    int index = 0;

    while (binaryToRegistry(&registryFromBinaryFile, binaryMetroFile) == BINARY_TO_REGISTRY_SUCESS)
    {
        if (registryFromBinaryFile.removido == IS_NOT_REMOVED)
        {

            int doesTheStationAlreadyExist = 0;
            for (int i = 0; i < index; i++)
            {
                if (strcmp(adjacencyList.listOfVertices[i].stationName, registryFromBinaryFile.nomeEstacao) == 0)
                {
                    doesTheStationAlreadyExist = 1;
                    break;
                }
            }

            if (doesTheStationAlreadyExist == 0)
            {

                vertex = createVertex(&registryFromBinaryFile);
                adjacencyList.listOfVertices[index] = vertex;

                index++;
            }

        }
    }

    qsort(adjacencyList.listOfVertices, index, sizeof(Vertex), compareVertexNameForQsort);

    fseek(binaryMetroFile, HEADER_SIZE, SEEK_SET);

    while (binaryToRegistry(&registryFromBinaryFile, binaryMetroFile) == BINARY_TO_REGISTRY_SUCESS)
    {
        if (registryFromBinaryFile.removido != IS_NOT_REMOVED)
            continue;

        Vertex *originVertex = binarySearchOnGraph(&adjacencyList, index, registryFromBinaryFile.nomeEstacao);
        if(originVertex == NULL)
            continue;
            
        if (registryFromBinaryFile.codProxEstacao != -1)
        {
            // busca pontual no arquivo pelo registro cujo codEstacao == codProxEstacao,
            // so para descobrir o NOME da estacao de destino
            Registry destRegistry;
            fseek(binaryMetroFile, HEADER_SIZE, SEEK_SET);
            while (binaryToRegistry(&destRegistry, binaryMetroFile) == BINARY_TO_REGISTRY_SUCESS)
            {
                if (destRegistry.codEstacao == registryFromBinaryFile.codProxEstacao)
                    break;
            }

            // procura se ja existe uma Connection para esse mesmo destino nesse vertice
            Connection *existing = originVertex->nextStation;
            Connection *found = NULL;
            while (existing != NULL)
            {
                if (strcmp(existing->connectionName, destRegistry.nomeEstacao) == 0)
                {
                    found = existing;
                    break;
                }
                existing = existing->nextConnection;
            }

            if (found != NULL)
            {
                addLineToConnection(found, registryFromBinaryFile.nomeLinha);
            }
            else
            {
                Registry tempReg = registryFromBinaryFile;
                // createConnection usa nomeEstacao/tamNomeEstacao do destino, entao
                // sobrescrevemos temporariamente com os dados resolvidos do destino
                tempReg.nomeEstacao = destRegistry.nomeEstacao;
                tempReg.tamNomeEstacao = destRegistry.tamNomeEstacao;
                tempReg.distProxEstacao = registryFromBinaryFile.distProxEstacao;
                tempReg.nomeLinha = registryFromBinaryFile.nomeLinha;

                connection = createConnection(&tempReg);
                insertConnectionOnVertex(originVertex, connection);
            }

            fseek(binaryMetroFile, HEADER_SIZE, SEEK_SET);
            // reposiciona no registro atual da passada externa (necessario apos o fseek interno)
            long currentPos = HEADER_SIZE + 0; // ajuste conforme seu esquema de RRN se necessario
        }

        // ---- aresta de integracao (codEstIntegra), so se nome for diferente ----
        if (registryFromBinaryFile.codEstIntegra != -1)
        {
            Registry integraRegistry;
            fseek(binaryMetroFile, HEADER_SIZE, SEEK_SET);
            while (binaryToRegistry(&integraRegistry, binaryMetroFile) == BINARY_TO_REGISTRY_SUCESS)
            {
                if (integraRegistry.codEstacao == registryFromBinaryFile.codEstIntegra)
                    break;
            }

            if (strcmp(integraRegistry.nomeEstacao, registryFromBinaryFile.nomeEstacao) != 0)
            {
                Registry tempReg = registryFromBinaryFile;
                tempReg.nomeEstacao = integraRegistry.nomeEstacao;
                tempReg.tamNomeEstacao = integraRegistry.tamNomeEstacao;
                tempReg.distProxEstacao = 0;
                tempReg.nomeLinha = "Integracao";

                connection = createConnection(&tempReg);
                insertConnectionOnVertex(originVertex, connection);
            }
        }
    }

    return CREATEGRAPH_SUCESS;
}

int calculateLowestDistanceFromStations(FILE *binaryMetroFile, char *originStationName, char *valueOriginStation, char *destinyStationName, char *value)
{
    ;
}

int optmizeRailingSystem(FILE *binaryMetroFile, char *nameStation, char *originValue)
{
    ;
}

int countCyclesBetweenStations(FILE *binaryMetroFile, char *nameStation, char *stationValue)
{
    ;
}

/*
    feat: adiciona struct Vertex
    fix: corrige leitura
    refactor: extrai busca da funcao propria
    docs: documentacao
    test: testes
    chore: manutencao, build, config
*/