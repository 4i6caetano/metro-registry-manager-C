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

  char **uniqueStations = malloc(5000 * sizeof(char *));
  int numUniqueStations = 0;

  Pair *uniquePairs = malloc(5000 * sizeof(Pair));
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
    int foundAtleastOne = 0; // a flag. if it finds atleast one match on the record, it is incremented.
    int m = 0; // The number of filters the user wants to aplly on this specific search.
    scanf("%d", &m);

    Field field[m]; // Creates an array of fields, the 'filters'.

    for (int j = 0; j < m; j++)
    {
      scanf("%s", field[j].nameOfTheField); // reads the target's name, like "codEstacao"
      ScanQuoteString(field[j].valueOfTheField); // captures the target value.
    }

    fseek(binaryFile, HEADER_SIZE, SEEK_SET); // Starting from header_size, defines the cursor after it.
    Registry registry;

    while (binaryToRegistry(&registry, binaryFile) == BINARY_TO_REGISTRY_SUCESS) //while binaryToRegistry converts the data of the FILE to our RAM is sucess, do it.
    {

      if (registry.removido == IS_NOT_REMOVED) // checks if removed
      {                  // if the registry is not removed
        int isEqual = 1; // 1 if positive, 0 if NOT positive
        // we search in it

        /* This drives the file pointer through the disk. It reads the 80-byte block, extract its variables from the FILE, allocate memory for variable-length strings,
        populates registry and skips the garbage bytes via SEEK_CUR. Returns SUCESS until hits EOF.*/
        for (int k = 0; k < m; k++)
        {

          /* compares each user filter against the fields of the active registries.*/

          if (strcmp(field[k].nameOfTheField, "codEstacao") == 0)
          {

            int searchValue = (strcmp(field[k].valueOfTheField, "") == 0) ? -1 : atoi(field[k].valueOfTheField);

            if (registry.codEstacao != searchValue)
            {
              isEqual = 0;
              break;
            }
          }

          else if (strcmp(field[k].nameOfTheField, "codLinha") == 0)
          {

            int searchValue = (strcmp(field[k].valueOfTheField, "") == 0) ? -1 : atoi(field[k].valueOfTheField);

            if (registry.codLinha != searchValue)
            {
              isEqual = 0;
              break;
            }
          }

          else if (strcmp(field[k].nameOfTheField, "codProxEstacao") == 0)
          {
            int searchValue = (strcmp(field[k].valueOfTheField, "") == 0) ? -1 : atoi(field[k].valueOfTheField);

            if (registry.codProxEstacao != searchValue)
            {
              isEqual = 0;
              break;
            }
          }

          else if (strcmp(field[k].nameOfTheField, "distProxEstacao") == 0)
          {
            int searchValue = (strcmp(field[k].valueOfTheField, "") == 0) ? -1 : atoi(field[k].valueOfTheField);

            if (registry.distProxEstacao != searchValue)
            {
              isEqual = 0;
              break;
            }
          }

          else if (strcmp(field[k].nameOfTheField, "codLinhaIntegra") == 0)
          {
            int searchValue = (strcmp(field[k].valueOfTheField, "") == 0) ? -1 : atoi(field[k].valueOfTheField);

            if (registry.codLinhaIntegra != searchValue)
            {
              isEqual = 0;
              break;
            }
          }

          else if (strcmp(field[k].nameOfTheField, "codEstIntegra") == 0)
          {
            int searchValue = (strcmp(field[k].valueOfTheField, "") == 0) ? -1 : atoi(field[k].valueOfTheField);
            if (registry.codEstIntegra != searchValue)
            {
              isEqual = 0;
              break;
            }
          }

          else if (strcmp(field[k].nameOfTheField, "nomeEstacao") == 0)
          {
            if (registry.tamNomeEstacao == 0)
            {
              if (strcmp(field[k].valueOfTheField, "") != 0)
              {
                isEqual = 0;
                break;
              }
            }
            else
            {
              if (strcmp(registry.nomeEstacao, field[k].valueOfTheField) != 0)
              {
                isEqual = 0;
                break;
              }
            }
          }

          else if (strcmp(field[k].nameOfTheField, "nomeLinha") == 0)
          {
            if (registry.tamNomeLinha == 0)
            {
              if (strcmp(field[k].valueOfTheField, "") != 0)
              {
                isEqual = 0;
                break;
              }
            }
            else
            {
              if (strcmp(registry.nomeLinha, field[k].valueOfTheField) != 0)
              {
                isEqual = 0;
                break;
              }
            }
          }

        } // closes the 'k' loop

        /* If the record survived the k loop without 'break', it satisfies the criteria. The program calls printRegistry()
        and increments our sucessful match counter. */

        if (isEqual == 1) 
        {
          printRegistry(&registry); // Print the registry found.
          foundAtleastOne++;
        }

      } // closes the registry verifier

      freeRegistry(&registry);

    } // closes the binaryToRegister function

    if (foundAtleastOne == 0)
    {
      printf("Registro inexistente.\n");
    }

    if (i < n - 1)
    {
      printf("\n");
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