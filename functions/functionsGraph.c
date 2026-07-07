#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "graph.h"
#include "functionsGraph.h"
#include "registry.h"
#include "header.h"

/* ------------------------------------------------------------------ */
/* Helpers                                                            */
/* ------------------------------------------------------------------ */

//checkFileStatus: lê o byte de status do cabeçalho e imprime falha se o arquivo estiver inconsistente
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

//buildGraph: constrói a lista de adjacência completa a partir dos registros do arquivo binário
//deve ser chamado após checkFileStatus já ter consumido o byte de status
static AdjacencyList buildGraph(FILE *f)
{
    //calcula quantos registros cabem no arquivo com base no tamanho total
    fseek(f, 0, SEEK_END);
    long fileSize   = ftell(f);
    int  totalSlots = (int)((fileSize - HEADER_SIZE) / REGISTRY_SIZE);

    if (totalSlots <= 0) totalSlots = 1;
    Registry *recs = (Registry *) malloc(sizeof(Registry) * totalSlots);
    int n = 0;
    fseek(f, HEADER_SIZE, SEEK_SET);
    //lê todos os registros do arquivo para a memória de uma vez
    while (n < totalSlots &&
           binaryToRegistry(&recs[n], f) == BINARY_TO_REGISTRY_SUCESS)
        n++;

    //passagem 1: coleta os nomes únicos de estações não removidas e cria os vértices
    //aloca totalSlots como limite superior para nunca estourar mesmo com nroEstacoes desatualizado
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
        //ignora registros removidos ou sem nome
        if (recs[i].removido != IS_NOT_REMOVED) continue;
        if (recs[i].tamNomeEstacao == 0)         continue;

        //verifica se o nome já foi adicionado como vértice para evitar duplicatas
        int dup = 0;
        for (int j = 0; j < vCount; j++)
            if (strcmp(list.listOfVertices[j].stationName, recs[i].nomeEstacao) == 0)
            { dup = 1; break; }

        if (!dup)
            list.listOfVertices[vCount++] = createVertex(&recs[i]);
    }

    //ordena os vértices por nome para possibilitar busca binária nas passagens seguintes
    qsort(list.listOfVertices, vCount, sizeof(Vertex), compareVertexNameForQsort);
    list.numberOfVertices = vCount;
    list.listOfVertices   = realloc(list.listOfVertices, sizeof(Vertex) * vCount);

    //passagem 2: adiciona as arestas entre os vértices
    for (int i = 0; i < n; i++)
    {
        Registry *r = &recs[i];
        //pula registros removidos ou sem nome
        if (r->removido != IS_NOT_REMOVED) continue;
        if (r->tamNomeEstacao == 0)         continue;

        Vertex *origin = binarySearchOnGraph(&list, vCount, r->nomeEstacao);
        if (origin == NULL) continue;

        //aresta normal: codEstacao -> codProxEstacao com nome da linha e distância
        if (r->codProxEstacao != -1 && r->nomeLinha != NULL)
        {
            const char *destName = NULL;
            //busca o registro de destino pelo código e verifica se ele não está removido
            for (int j = 0; j < n; j++)
                if (recs[j].codEstacao == r->codProxEstacao &&
                    recs[j].tamNomeEstacao > 0 &&
                    recs[j].removido == IS_NOT_REMOVED)
                { destName = recs[j].nomeEstacao; break; }

            if (destName != NULL)
            {
                //se já existe conexão para esse destino, apenas adiciona a linha à conexão
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

        //aresta de integração: codEstacao -> codEstIntegra com distância 0 e linha "Integração"
        if (r->codEstIntegra != -1)
        {
            const char *integName = NULL;
            //busca o registro de integração pelo código e verifica se ele não está removido
            for (int j = 0; j < n; j++)
                if (recs[j].codEstacao == r->codEstIntegra &&
                    recs[j].tamNomeEstacao > 0 &&
                    recs[j].removido == IS_NOT_REMOVED)
                { integName = recs[j].nomeEstacao; break; }

            //evita auto-laço e insere apenas uma aresta de integração por par de estações
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

    for (int i = 0; i < n; i++) freeRegistry(&recs[i]);
    free(recs);

    return list;
}

//findIdx: retorna o índice do vértice com o nome dado, ou -1 se não encontrado
static int findIdx(AdjacencyList *list, const char *name)
{
    Vertex *v = binarySearchOnGraph(list, list->numberOfVertices, name);
    if (v == NULL) return -1;
    return (int)(v - list->listOfVertices);
}

/* ================================================================== */
/* [10] Build graph and print adjacency list                          */
/* ================================================================== */

//função 10: constrói o grafo a partir do arquivo binário e imprime a lista de adjacência
int createGraphFromMetro(FILE *binaryMetroFile)
{
    //verifica consistência do arquivo antes de prosseguir
    if (!binaryMetroFile || !checkFileStatus(binaryMetroFile))
        return CREATEGRAPH_FAILURE;

    AdjacencyList list = buildGraph(binaryMetroFile);
    printAdjacencyList(&list);
    freeAdjacencyList(&list);
    return CREATEGRAPH_SUCESS;
}

/* ================================================================== */
/* [11] Dijkstra: shortest path between two stations                  */
/* ================================================================== */

//função 11: calcula o menor caminho entre duas estações usando o algoritmo de dijkstra
//em caso de empate de distância, desempata pelo menor nome lexicográfico da estação predecessora
int calculateLowestDistanceFromStations(FILE *binaryMetroFile,
                                        const char *originField,
                                        const char *originValue,
                                        const char *destField,
                                        const char *destValue)
{
    (void)originField; (void)destField;

    //verifica consistência do arquivo antes de prosseguir
    if (!binaryMetroFile || !checkFileStatus(binaryMetroFile))
        return CREATEGRAPH_FAILURE;

    AdjacencyList list = buildGraph(binaryMetroFile);
    int n   = list.numberOfVertices;
    int src = findIdx(&list, originValue);
    int dst = findIdx(&list, destValue);

    //verifica se as estações de origem e destino existem no grafo
    if (src == -1 || dst == -1)
    {
        printf("Falha na execução da funcionalidade.\n");
        freeAdjacencyList(&list);
        return CREATEGRAPH_FAILURE;
    }

    //dist: menor distância conhecida até cada vértice
    //prev: índice do predecessor no caminho mínimo
    //visited: marca vértices já processados
    int *dist    = (int *) malloc(n * sizeof(int));
    int *prev    = (int *) malloc(n * sizeof(int));
    int *visited = (int *) calloc(n, sizeof(int));

    //inicializa todas as distâncias como infinito e predecessores como inexistentes
    for (int i = 0; i < n; i++) { dist[i] = INT_MAX; prev[i] = -1; }
    dist[src] = 0;

    for (int iter = 0; iter < n; iter++)
    {
        //escolhe o vértice não visitado de menor distância; empate desfeito pelo menor nome
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

        //relaxa todas as arestas saindo de u
        Connection *c = list.listOfVertices[u].nextStation;
        while (c != NULL)
        {
            int v = findIdx(&list, c->connectionName);
            if (v != -1 && !visited[v])
            {
                int nd = dist[u] + c->distanceOfNextStation;
                //atualiza se encontrou caminho mais curto; empate resolvido pelo menor nome do predecessor
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
        //reconstrói o caminho percorrendo os predecessores do destino até a origem
        int *path = (int *) malloc(n * sizeof(int));
        int  pLen = 0;
        for (int cur = dst; cur != -1; cur = prev[cur])
            path[pLen++] = cur;

        //inverte o array para ficar na ordem origem -> destino
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

/* ================================================================== */
/* [12] Prim's MST + DFS traversal                                    */
/* ================================================================== */

//edgeWeight: retorna o menor peso de aresta entre u e v em qualquer direção, INT_MAX se não existir
//trata o grafo como não dirigido verificando u->v e v->u
static int edgeWeight(AdjacencyList *list, int u, int v)
{
    const char *vName = list->listOfVertices[v].stationName;
    const char *uName = list->listOfVertices[u].stationName;
    int w = INT_MAX;

    //verifica aresta u -> v
    Connection *c = list->listOfVertices[u].nextStation;
    while (c)
    {
        if (strcmp(c->connectionName, vName) == 0 && c->distanceOfNextStation < w)
            w = c->distanceOfNextStation;
        c = c->nextConnection;
    }

    //verifica aresta v -> u
    c = list->listOfVertices[v].nextStation;
    while (c)
    {
        if (strcmp(c->connectionName, uName) == 0 && c->distanceOfNextStation < w)
            w = c->distanceOfNextStation;
        c = c->nextConnection;
    }

    return w;
}

//dfsMST: percorre recursivamente a árvore geradora mínima em profundidade e imprime cada aresta pai->filho
//os filhos de cada nó são ordenados pelo nome antes de serem visitados
static void dfsMST(AdjacencyList *list, int *parent, int *key, int n, int u)
{
    //coleta todos os filhos do vértice u na árvore geradora
    int *ch  = (int *) malloc(n * sizeof(int));
    int  nCh = 0;
    for (int v = 0; v < n; v++)
        if (parent[v] == u) ch[nCh++] = v;

    //ordena os filhos pelo nome da estação para saída determinística
    for (int i = 1; i < nCh; i++)
    {
        int tmp = ch[i], j = i - 1;
        while (j >= 0 && strcmp(list->listOfVertices[ch[j]].stationName,
                                list->listOfVertices[tmp].stationName) > 0)
        { ch[j+1] = ch[j]; j--; }
        ch[j+1] = tmp;
    }

    //imprime a aresta pai->filho e desce recursivamente em cada filho
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

//função 12: calcula a árvore geradora mínima pelo algoritmo de prim e imprime as arestas via dfs
//em caso de empate de peso, desempata pelo menor nome lexicográfico da estação pai
int optimizeRailingSystem(FILE *binaryMetroFile,
                          const char *nameStation,
                          const char *originValue)
{
    (void)nameStation;

    //verifica consistência do arquivo antes de prosseguir
    if (!binaryMetroFile || !checkFileStatus(binaryMetroFile))
        return CREATEGRAPH_FAILURE;

    AdjacencyList list = buildGraph(binaryMetroFile);
    int n   = list.numberOfVertices;
    int src = findIdx(&list, originValue);

    //verifica se a estação de origem existe no grafo
    if (src == -1)
    {
        printf("Falha na execução da funcionalidade.\n");
        freeAdjacencyList(&list);
        return CREATEGRAPH_FAILURE;
    }

    //key: menor peso de aresta que conecta o vértice à árvore
    //parent: índice do pai de cada vértice na árvore geradora
    //inMST: marca vértices já incluídos na árvore
    int *key    = (int *) malloc(n * sizeof(int));
    int *parent = (int *) malloc(n * sizeof(int));
    int *inMST  = (int *) calloc(n, sizeof(int));

    //inicializa as chaves como infinito; a origem entra com chave 0
    for (int i = 0; i < n; i++) { key[i] = INT_MAX; parent[i] = -1; }
    key[src] = 0;

    for (int iter = 0; iter < n; iter++)
    {
        //escolhe o vértice fora da mst com menor chave; empate desfeito pelo menor nome
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

        //atualiza a chave dos vizinhos não incluídos na mst
        for (int v = 0; v < n; v++)
        {
            if (inMST[v]) continue;
            int w = edgeWeight(&list, u, v);
            if (w == INT_MAX) continue;

            //atualiza se o peso for menor; empate resolvido pelo menor nome do pai
            int update = 0;
            if (w < key[v]) update = 1;
            else if (w == key[v] && parent[v] != -1 &&
                     strcmp(list.listOfVertices[u].stationName,
                            list.listOfVertices[parent[v]].stationName) < 0)
                update = 1;

            if (update) { key[v] = w; parent[v] = u; }
        }
    }

    //percorre a árvore em profundidade a partir da origem para imprimir as arestas
    dfsMST(&list, parent, key, n, src);

    free(key); free(parent); free(inMST);
    freeAdjacencyList(&list);
    return CREATEGRAPH_SUCESS;
}

/* ================================================================== */
/* [13] Count simple cycles from origin station                       */
/* ================================================================== */

//CycleCtx: contexto compartilhado entre as chamadas recursivas da dfs de contagem de ciclos
typedef struct
{
    AdjacencyList *list;    //grafo sendo percorrido
    int           *visited; //controla quais vértices estão no caminho atual
    int            origin;  //índice da estação de origem (ponto de retorno do ciclo)
    int            count;   //quantidade de ciclos simples encontrados até agora
} CycleCtx;

//dfsCycles: dfs com backtracking que conta quantos caminhos simples retornam à estação de origem
static void dfsCycles(CycleCtx *ctx, int v)
{
    Connection *c = ctx->list->listOfVertices[v].nextStation;
    while (c != NULL)
    {
        int w = findIdx(ctx->list, c->connectionName);
        if (w != -1)
        {
            if (w == ctx->origin)
                ctx->count++;             //chegou de volta à origem: ciclo simples completo
            else if (!ctx->visited[w])
            {
                ctx->visited[w] = 1;
                dfsCycles(ctx, w);
                ctx->visited[w] = 0;      //backtrack para explorar outros caminhos
            }
        }
        c = c->nextConnection;
    }
}

//função 13: conta todos os ciclos simples que partem e retornam à estação informada
//imprime -1 se não existirem ciclos, conforme especificação
int countCyclesBetweenStations(FILE *binaryMetroFile,
                               const char *nameStation,
                               const char *stationValue)
{
    (void)nameStation;

    //verifica consistência do arquivo antes de prosseguir
    if (!binaryMetroFile || !checkFileStatus(binaryMetroFile))
        return CREATEGRAPH_FAILURE;

    AdjacencyList list = buildGraph(binaryMetroFile);
    int n   = list.numberOfVertices;
    int src = findIdx(&list, stationValue);

    //verifica se a estação de origem existe no grafo
    if (src == -1)
    {
        printf("Falha na execução da funcionalidade.\n");
        freeAdjacencyList(&list);
        return CREATEGRAPH_FAILURE;
    }

    int *visited = (int *) calloc(n, sizeof(int));
    //marca a origem como visitada para que os caminhos não voltem por ela durante a busca
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
