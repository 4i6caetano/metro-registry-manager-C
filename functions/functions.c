#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "functions.h"
#include "registry.h"
#include "header.h"
#include "utils.h"

int csvToBinary(FILE *inputCSVFile, FILE *binaryFile)
{
  char buffer[8000];
  Registry newRegistry; 

    /* Initializates the buffer on which we'll temporarily store our information, and the Registry that will receive it.*/

  if (inputCSVFile == NULL || binaryFile == NULL)
  {
    return FUNCTION_FAILURE; // error handling.
  }

  writeInitialHeader(binaryFile); /* writes the first memory allocation, the Header of the File*/
  int rrnCounter = 0;

  /* conta quantas linhas de dados existem no CSV para alocar os arrays de
     únicos com capacidade suficiente (cada linha contribui no máximo
     1 estação única e 1 par único), evitando um array de tamanho fixo */
  int maxPossibleUniques = 0;
  fgets(buffer, sizeof(buffer), inputCSVFile); // pula a linha de cabeçalho
  while (fgets(buffer, sizeof(buffer), inputCSVFile) != NULL)
    maxPossibleUniques++;
  if (maxPossibleUniques == 0) maxPossibleUniques = 1;
  fseek(inputCSVFile, 0, SEEK_SET);

  char **uniqueStations = malloc(maxPossibleUniques * sizeof(char *));
  int numUniqueStations = 0;

  Pair *uniquePairs = malloc(maxPossibleUniques * sizeof(Pair));
  int numUniquePairs = 0;

  fgets(buffer, sizeof(buffer), inputCSVFile); // reads the Header line and skips it.

  while (fgets(buffer, sizeof(buffer), inputCSVFile) != NULL) // While fgets can read the lines, it continuously keep doing that
  {
    newRegistry.nomeEstacao = NULL;
    newRegistry.nomeLinha = NULL;

    fillRegistry(buffer, &newRegistry); // fills the registry with that data

    if (newRegistry.tamNomeEstacao > 0)
    {
      addUniqueStation(uniqueStations, &numUniqueStations, newRegistry.nomeEstacao);
    }
    addUniquePair(uniquePairs, &numUniquePairs, newRegistry.codEstacao, newRegistry.codProxEstacao);

    registryToBinary(&newRegistry, binaryFile);
    rrnCounter++;
  } /* This works because fillRegistry used the sizes to determine what to save.*/

  updateFinalHeader(binaryFile, rrnCounter, numUniqueStations, numUniquePairs); //Ultimately, update the header so it can be up to date.

  for (int i = 0; i < numUniqueStations; i++)
    free(uniqueStations[i]);
  free(uniqueStations);
  free(uniquePairs);

  return FUNCTION_SUCESS; // SUCESS
}

int displayValidRecords(FILE *binaryFile)
{

  Registry newRegistry;
  int validRegistry = 0;

  if (binaryFile == NULL)
  {
    printf("Falha no processamento do arquivo.\n"); // Error handling
    return FUNCTION_FAILURE;
  }

  fseek(binaryFile, HEADER_SIZE, SEEK_SET);  /* points to the beggining of the file*/

  while (binaryToRegistry(&newRegistry, binaryFile) == BINARY_TO_REGISTRY_SUCESS) // while binaryToRegistry can read and convert, keep going
  {
    if (newRegistry.removido == IS_NOT_REMOVED)
    {
      printRegistry(&newRegistry);
      validRegistry++; // adds validRegistry if its not removed
    }

    freeRegistry(&newRegistry); // frees the actual cursor.
  }

  if (validRegistry == 0)
  {
    printf("Registro inexistente.\n"); // Case there's no valid Registries.
  }

  return FUNCTION_SUCESS; // SUCESS
}

void searchData(FILE *binaryFile, int n) // Takes the FILE and the number of independent searches the user wants.
{

  if (binaryFile == NULL)
  {
    printf("Falha no processamento do arquivo.\n"); // Error handling
    return;
  }

  for (int i = 0; i < n; i++) // This loop is responsible for each independent search the user asked for, defined by 'n'.
  {
    int m = 0; // The number of filters the user wants to aplly on this specific search.
    scanf("%d", &m);

    Field field[m]; // Creates an array of fields, the 'filters'.

    for (int j = 0; j < m; j++)
    {
      scanf("%s", field[j].nameOfTheField); // reads the target's name, like "codEstacao"
      ScanQuoteString(field[j].valueOfTheField); // captures the target value.
    }

    fseek(binaryFile, HEADER_SIZE, SEEK_SET); // Starting from header_size, defines the cursor after it.

    Registry temporaryRegistry;

    int registersThatFulfillTheSearch = 0; // counter for each search

    sequentialSearchInRegister(temporaryRegistry, binaryFile, m, field, &registersThatFulfillTheSearch);

    if (registersThatFulfillTheSearch == 0)
    {
      printf("Registro inexistente.\n");
    }
  } // closes the 'i' loop.
} // closes the searchData function.

int searchByRRN(FILE *binaryFile, int RRN)
{
  if (binaryFile == NULL)
  {
    printf("Falha no processamento do arquivo.\n");
    return FUNCTION_FAILURE;
  }

  long offset = HEADER_SIZE + (RRN * 80);

  fseek(binaryFile, offset, SEEK_SET);

  Registry registry;

  if (binaryToRegistry(&registry, binaryFile) == BINARY_TO_REGISTRY_SUCESS)
  {

    if (registry.removido == IS_NOT_REMOVED)
    {
      printRegistry(&registry);
      freeRegistry(&registry);
      return FUNCTION_SUCESS;
    }
    else
    {
      printf("Registro inexistente.\n");
      freeRegistry(&registry);
      return FUNCTION_FAILURE;
    }
  }

  else
  {
    printf("Registro inexistente.\n");
    return FUNCTION_FAILURE;
  }
}