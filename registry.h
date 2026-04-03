#ifndef REGISTRY_H
#define REGISTRY_H

#define IS_REMOVED 1
#define IS_NOT_REMOVED -1

#define DATA_REGISTRY_SIZE 80

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct registry {

  char removido;
  int proximo;

  int codEstacao;
  int codLinha;
  int codProxEstacao;
  int distProxEstacao;
  int codLinhaIntegra;
  int codEstIntegra;

  int tamNomeEstacao;
  char* nomeEstacao;

  int tamNomeLinha;
  char* nomeLinha;

} Registry;




#endif