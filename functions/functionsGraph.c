#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "graph.h"
#include "functionsGraph.h"
#include "registry.h"
#include "header.h"

static int checkFileStatus(FILE *f)
{
    char status;
    fread(&status, sizeof(char), 1, f);
    if (status == STATUS_INCONSISTENT)
    {
        printf("Falha na execução da funcionalidade.\n");
        return 0;
    }
    return 1;
}

// monta a lista de adjacência a partir dos registros do arquivo binário
// (assume que checkFileStatus já consumiu o byte de status)
static AdjacencyList buildGraph(FILE *f)
{
    fseek(f, 0, SEEK_END);
    long fileSize   = ftell(f);
    int  totalSlots = (int)((fileSize - HEADER_SIZE) / REGISTRY_SIZE);

    if (totalSlots <= 0) totalSlots = 1;
    Registry *recs = (Registry *) malloc(sizeof(Registry) * totalSlots);
    int n = 0;
    fseek(f, HEADER_SIZE, SEEK_SET);
    while (n < totalSlots &&
           binaryToRegistry(&recs[n], f) == BINARY_TO_REGISTRY_SUCESS)
        n++;

    // cria um vértice pra cada nome de estação único, ignorando removidos
    AdjacencyList list;
    list.numberOfVertices = totalSlots;
    list.listOfVertices   = (Vertex *) malloc(sizeof(Vertex) * totalSlots);
    for (int i = 0; i < totalSlots; i++)
    {
        list.listOfVertices[i].stationName = NULL;
        list.listOfVertices[i].nextStation = NULL;
    }
    int vCount = 0;

    for (int i = 0; i < n; i++)
    {
        if (recs[i].removido != IS_NOT_REMOVED) continue;
        if (recs[i].tamNomeEstacao == 0)         continue;

        int dup = 0;
        for (int j = 0; j < vCount; j++)
            if (strcmp(list.listOfVertices[j].stationName, recs[i].nomeEstacao) == 0)
            { dup = 1; break; }

        if (!dup)
            list.listOfVertices[vCount++] = createVertex(&recs[i]);
    }

    // ordena por nome pra dar pra usar busca binária depois
    qsort(list.listOfVertices, vCount, sizeof(Vertex), compareVertexNameForQsort);
    list.numberOfVertices = vCount;
    list.listOfVertices   = realloc(list.listOfVertices, sizeof(Vertex) * vCount);

    for (int i = 0; i < n; i++)
    {
        Registry *r = &recs[i];
        if (r->removido != IS_NOT_REMOVED) continue;
        if (r->tamNomeEstacao == 0)         continue;

        Vertex *origin = binarySearchOnGraph(&list, vCount, r->nomeEstacao);
        if (origin == NULL) continue;

        // aresta normal: codEstacao -> codProxEstacao
        if (r->codProxEstacao != -1 && r->nomeLinha != NULL)
        {
            const char *destName = NULL;
            for (int j = 0; j < n; j++)
                if (recs[j].codEstacao == r->codProxEstacao &&
                    recs[j].tamNomeEstacao > 0 &&
                    recs[j].removido == IS_NOT_REMOVED)
                { destName = recs[j].nomeEstacao; break; }

            if (destName != NULL)
            {
                Connection *existing = origin->nextStation, *found = NULL;
                while (existing)
                {
                    if (strcmp(existing->connectionName, destName) == 0)
                    { found = existing; break; }
                    existing = existing->nextConnection;
                }

                if (found) addLineToConnection(found, r->nomeLinha);
                else
                {
                    Connection *c = createConnection(destName,
                                                     r->distProxEstacao,
                                                     r->nomeLinha);
                    insertConnectionOnVertex(origin, c);
                }
            }
        }

        // aresta de integração: distância 0, linha "Integração"
        if (r->codEstIntegra != -1)
        {
            const char *integName = NULL;
            for (int j = 0; j < n; j++)
                if (recs[j].codEstacao == r->codEstIntegra &&
                    recs[j].tamNomeEstacao > 0 &&
                    recs[j].removido == IS_NOT_REMOVED)
                { integName = recs[j].nomeEstacao; break; }

            if (integName != NULL &&
                strcmp(integName, r->nomeEstacao) != 0)
            {
                Connection *existing = origin->nextStation, *found = NULL;
                while (existing)
                {
                    if (strcmp(existing->connectionName, integName) == 0 &&
                        existing->distanceOfNextStation == 0)
                    { found = existing; break; }
                    existing = existing->nextConnection;
                }
                if (!found)
                {
                    Connection *c = createConnection(integName, 0, "Integração");
                    insertConnectionOnVertex(origin, c);
                }
            }
        }
    }

    // estações sem nenhuma conexão de saída não entram na lista final
    int compactCount = 0;
    for (int i = 0; i < vCount; i++)
    {
        if (list.listOfVertices[i].nextStation != NULL)
            list.listOfVertices[compactCount++] = list.listOfVertices[i];
        else
            free(list.listOfVertices[i].stationName);
    }
    vCount = compactCount;
    list.numberOfVertices = vCount;
    list.listOfVertices   = realloc(list.listOfVertices, sizeof(Vertex) * vCount);

    for (int i = 0; i < n; i++) freeRegistry(&recs[i]);
    free(recs);

    return list;
}

static int findIdx(AdjacencyList *list, const char *name)
{
    Vertex *v = binarySearchOnGraph(list, list->numberOfVertices, name);
    if (v == NULL) return -1;
    return (int)(v - list->listOfVertices);
}

int createGraphFromMetro(FILE *binaryMetroFile)
{
    if (!binaryMetroFile || !checkFileStatus(binaryMetroFile))
        return CREATEGRAPH_FAILURE;

    AdjacencyList list = buildGraph(binaryMetroFile);
    printAdjacencyList(&list);
    freeAdjacencyList(&list);
    return CREATEGRAPH_SUCESS;
}

// dijkstra; em empate de distância, desempata pelo nome do predecessor
int calculateLowestDistanceFromStations(FILE *binaryMetroFile,
                                        const char *originField,
                                        const char *originValue,
                                        const char *destField,
                                        const char *destValue)
{
    (void)originField; (void)destField;

    if (!binaryMetroFile || !checkFileStatus(binaryMetroFile))
        return CREATEGRAPH_FAILURE;

    AdjacencyList list = buildGraph(binaryMetroFile);
    int n   = list.numberOfVertices;
    int src = findIdx(&list, originValue);
    int dst = findIdx(&list, destValue);

    if (src == -1 || dst == -1)
    {
        printf("Falha na execução da funcionalidade.\n");
        freeAdjacencyList(&list);
        return CREATEGRAPH_FAILURE;
    }

    int *dist    = (int *) malloc(n * sizeof(int));
    int *prev    = (int *) malloc(n * sizeof(int));
    int *visited = (int *) calloc(n, sizeof(int));

    for (int i = 0; i < n; i++) { dist[i] = INT_MAX; prev[i] = -1; }
    dist[src] = 0;

    for (int iter = 0; iter < n; iter++)
    {
        int u = -1;
        for (int i = 0; i < n; i++)
        {
            if (visited[i]) continue;
            if (u == -1 || dist[i] < dist[u] ||
                (dist[i] == dist[u] &&
                 strcmp(list.listOfVertices[i].stationName,
                        list.listOfVertices[u].stationName) < 0))
                u = i;
        }
        if (u == -1 || dist[u] == INT_MAX) break;
        visited[u] = 1;
        if (u == dst) break;

        Connection *c = list.listOfVertices[u].nextStation;
        while (c != NULL)
        {
            int v = findIdx(&list, c->connectionName);
            if (v != -1 && !visited[v])
            {
                int nd = dist[u] + c->distanceOfNextStation;
                int update = 0;
                if (nd < dist[v]) update = 1;
                else if (nd == dist[v] && prev[v] != -1 &&
                         strcmp(list.listOfVertices[u].stationName,
                                list.listOfVertices[prev[v]].stationName) < 0)
                    update = 1;

                if (update) { dist[v] = nd; prev[v] = u; }
            }
            c = c->nextConnection;
        }
    }

    if (dist[dst] == INT_MAX)
    {
        printf("Não existe caminho entre as estações solicitadas.\n");
    }
    else
    {
        int *path = (int *) malloc(n * sizeof(int));
        int  pLen = 0;
        for (int cur = dst; cur != -1; cur = prev[cur])
            path[pLen++] = cur;

        for (int i = 0; i < pLen / 2; i++)
        { int t = path[i]; path[i] = path[pLen-1-i]; path[pLen-1-i] = t; }

        printf("Numero de estacoes que serao percorridas: %d\n", pLen - 1);
        printf("Distancia que sera percorrida: %d\n", dist[dst]);
        for (int i = 0; i < pLen; i++)
        {
            if (i > 0) printf(", ");
            printf("%s", list.listOfVertices[path[i]].stationName);
        }
        printf("\n");
        free(path);
    }

    free(dist); free(prev); free(visited);
    freeAdjacencyList(&list);
    return CREATEGRAPH_SUCESS;
}

static int edgeWeight(AdjacencyList *list, int u, int v)
{
    const char *vName = list->listOfVertices[v].stationName;
    const char *uName = list->listOfVertices[u].stationName;
    int w = INT_MAX;

    Connection *c = list->listOfVertices[u].nextStation;
    while (c)
    {
        if (strcmp(c->connectionName, vName) == 0 && c->distanceOfNextStation < w)
            w = c->distanceOfNextStation;
        c = c->nextConnection;
    }

    c = list->listOfVertices[v].nextStation;
    while (c)
    {
        if (strcmp(c->connectionName, uName) == 0 && c->distanceOfNextStation < w)
            w = c->distanceOfNextStation;
        c = c->nextConnection;
    }

    return w;
}

static void dfsMST(AdjacencyList *list, int *parent, int *key, int n, int u)
{
    int *ch  = (int *) malloc(n * sizeof(int));
    int  nCh = 0;
    for (int v = 0; v < n; v++)
        if (parent[v] == u) ch[nCh++] = v;

    // ordena os filhos pelo nome pra saída ficar determinística
    for (int i = 1; i < nCh; i++)
    {
        int tmp = ch[i], j = i - 1;
        while (j >= 0 && strcmp(list->listOfVertices[ch[j]].stationName,
                                list->listOfVertices[tmp].stationName) > 0)
        { ch[j+1] = ch[j]; j--; }
        ch[j+1] = tmp;
    }

    for (int i = 0; i < nCh; i++)
    {
        int v = ch[i];
        printf("%s, %s, %d\n",
               list->listOfVertices[u].stationName,
               list->listOfVertices[v].stationName,
               key[v]);
        dfsMST(list, parent, key, n, v);
    }
    free(ch);
}

// prim + dfs pra imprimir as arestas da mst
int optimizeRailingSystem(FILE *binaryMetroFile,
                          const char *nameStation,
                          const char *originValue)
{
    (void)nameStation;

    if (!binaryMetroFile || !checkFileStatus(binaryMetroFile))
        return CREATEGRAPH_FAILURE;

    AdjacencyList list = buildGraph(binaryMetroFile);
    int n   = list.numberOfVertices;
    int src = findIdx(&list, originValue);

    if (src == -1)
    {
        printf("Falha na execução da funcionalidade.\n");
        freeAdjacencyList(&list);
        return CREATEGRAPH_FAILURE;
    }

    int *key    = (int *) malloc(n * sizeof(int));
    int *parent = (int *) malloc(n * sizeof(int));
    int *inMST  = (int *) calloc(n, sizeof(int));

    for (int i = 0; i < n; i++) { key[i] = INT_MAX; parent[i] = -1; }
    key[src] = 0;

    for (int iter = 0; iter < n; iter++)
    {
        int u = -1;
        for (int i = 0; i < n; i++)
        {
            if (inMST[i]) continue;
            if (u == -1 || key[i] < key[u] ||
                (key[i] == key[u] &&
                 strcmp(list.listOfVertices[i].stationName,
                        list.listOfVertices[u].stationName) < 0))
                u = i;
        }
        if (u == -1 || key[u] == INT_MAX) break;
        inMST[u] = 1;

        for (int v = 0; v < n; v++)
        {
            if (inMST[v]) continue;
            int w = edgeWeight(&list, u, v);
            if (w == INT_MAX) continue;

            int update = 0;
            if (w < key[v]) update = 1;
            else if (w == key[v] && parent[v] != -1 &&
                     strcmp(list.listOfVertices[u].stationName,
                            list.listOfVertices[parent[v]].stationName) < 0)
                update = 1;

            if (update) { key[v] = w; parent[v] = u; }
        }
    }

    dfsMST(&list, parent, key, n, src);

    free(key); free(parent); free(inMST);
    freeAdjacencyList(&list);
    return CREATEGRAPH_SUCESS;
}

typedef struct
{
    AdjacencyList *list;
    int           *visited;
    int            origin;
    int            count;
} CycleCtx;

static void dfsCycles(CycleCtx *ctx, int v)
{
    Connection *c = ctx->list->listOfVertices[v].nextStation;
    while (c != NULL)
    {
        int w = findIdx(ctx->list, c->connectionName);
        if (w != -1)
        {
            if (w == ctx->origin)
                ctx->count++;
            else if (!ctx->visited[w])
            {
                ctx->visited[w] = 1;
                dfsCycles(ctx, w);
                ctx->visited[w] = 0;
            }
        }
        c = c->nextConnection;
    }
}

// conta os ciclos simples que saem e voltam pra stationValue; imprime -1 se não houver nenhum
int countCyclesBetweenStations(FILE *binaryMetroFile,
                               const char *nameStation,
                               const char *stationValue)
{
    (void)nameStation;

    if (!binaryMetroFile || !checkFileStatus(binaryMetroFile))
        return CREATEGRAPH_FAILURE;

    AdjacencyList list = buildGraph(binaryMetroFile);
    int n   = list.numberOfVertices;
    int src = findIdx(&list, stationValue);

    if (src == -1)
    {
        printf("Falha na execução da funcionalidade.\n");
        freeAdjacencyList(&list);
        return CREATEGRAPH_FAILURE;
    }

    int *visited = (int *) calloc(n, sizeof(int));
    visited[src] = 1;

    CycleCtx ctx = { &list, visited, src, 0 };
    dfsCycles(&ctx, src);

    if (ctx.count == 0)
        printf("Quantidade de ciclos: -1\n");
    else
        printf("Quantidade de ciclos: %d\n", ctx.count);

    free(visited);
    freeAdjacencyList(&list);
    return CREATEGRAPH_SUCESS;
}
