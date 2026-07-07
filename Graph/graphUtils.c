#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "graphUtils.h"
#include "registry.h"


Vertex createVertex(Registry *reg)
{
    Vertex v;
    v.stationName = (char *) malloc(reg->tamNomeEstacao + 1);
    strcpy(v.stationName, reg->nomeEstacao);
    v.nextStation = NULL;
    return v;
}

Connection *createConnection(const char *destName, int dist, const char *lineName)
{
    Connection *c = (Connection *) malloc(sizeof(Connection));

    c->connectionName        = (char *) malloc(strlen(destName) + 1);
    strcpy(c->connectionName, destName);

    c->distanceOfNextStation = dist;
    c->numberOfLines         = 1;
    c->lines                 = (char **) malloc(sizeof(char *));
    c->lines[0]              = (char *) malloc(strlen(lineName) + 1);
    strcpy(c->lines[0], lineName);

    c->nextConnection = NULL;
    return c;
}

void addLineToConnection(Connection *c, const char *lineName)
{
    for (int i = 0; i < c->numberOfLines; i++)
        if (strcmp(c->lines[i], lineName) == 0) return; //evita duplicatas

    c->numberOfLines++;
    c->lines = (char **) realloc(c->lines, sizeof(char *) * c->numberOfLines);
    c->lines[c->numberOfLines - 1] = (char *) malloc(strlen(lineName) + 1);
    strcpy(c->lines[c->numberOfLines - 1], lineName);

    /* keep lines sorted ascending */
    for (int i = c->numberOfLines - 1; i > 0; i--)
    {
        if (strcmp(c->lines[i], c->lines[i - 1]) < 0)
        {
            char *tmp        = c->lines[i];
            c->lines[i]     = c->lines[i - 1];
            c->lines[i - 1] = tmp;
        }
        else break;
    }
}

void insertConnectionOnVertex(Vertex *vertex, Connection *conn)
{
    Connection *prev = NULL;
    Connection *cur  = vertex->nextStation;

    while (cur != NULL)
    {
        int cmp = strcmp(cur->connectionName, conn->connectionName);
        if (cmp > 0) break;                      /* insert before cur */
        if (cmp == 0)
        {
            /* same destination: tie-break on first line name */
            if (strcmp(cur->lines[0], conn->lines[0]) > 0) break;
        }
        prev = cur;
        cur  = cur->nextConnection;
    }

    conn->nextConnection = cur;
    if (prev == NULL) vertex->nextStation   = conn;
    else              prev->nextConnection  = conn;
}

int compareVertexNameForQsort(const void *a, const void *b)
{
    return strcmp(((const Vertex *) a)->stationName,
                  ((const Vertex *) b)->stationName);
}

Vertex *binarySearchOnGraph(AdjacencyList *list, int n, const char *name)
{
    int lo = 0, hi = n - 1;
    while (lo <= hi)
    {
        int mid = lo + (hi - lo) / 2;
        int cmp = strcmp(list->listOfVertices[mid].stationName, name);
        if (cmp == 0) return &list->listOfVertices[mid];
        if (cmp  < 0) lo = mid + 1;
        else          hi = mid - 1;
    }
    return NULL;
}

void printAdjacencyList(AdjacencyList *list)
{
    for (int i = 0; i < list->numberOfVertices; i++)
    {
        Vertex *v = &list->listOfVertices[i];
        printf("%s", v->stationName);

        Connection *c = v->nextStation;
        while (c != NULL)
        {
            printf(", %s, %d", c->connectionName, c->distanceOfNextStation);
            for (int l = 0; l < c->numberOfLines; l++)
                printf(", %s", c->lines[l]);
            c = c->nextConnection;
        }
        printf("\n");
    }
}

void freeAdjacencyList(AdjacencyList *list)
{
    for (int i = 0; i < list->numberOfVertices; i++)
    {
        free(list->listOfVertices[i].stationName);

        Connection *c = list->listOfVertices[i].nextStation;
        while (c != NULL)
        {
            Connection *next = c->nextConnection;
            free(c->connectionName);
            for (int l = 0; l < c->numberOfLines; l++) free(c->lines[l]);
            free(c->lines);
            free(c);
            c = next;
        }
    }
    free(list->listOfVertices);
}
