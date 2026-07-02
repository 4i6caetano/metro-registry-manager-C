#include <stdio.h>

#include "functionsGraph.h"

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

int createGraphFromMetro(FILE* binaryMetroFile)
{
  // ler o header e descobrir quantas estacoes unicas tem. cada estacao sera um vertice.
  // alocar a Adjacencia com esse tamanho
}

int calculateLowestDistanceFromStations(FILE* binaryMetroFile, char* originStationName, char* valueOriginStation, char* destinyStationName, char* value)
{
    ;
}

int optmizeRailingSystem(FILE* binaryMetroFile, char* nameStation, char* originValue)
{
    ;
}

int countCyclesBetweenStations(FILE* binaryMetroFile, char* nameStation, char* stationValue)
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