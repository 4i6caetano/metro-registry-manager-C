#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"
#include "registry.h"
#include "utils.h"
#include "functions.h"

typedef enum function
{
  _,             /**<void field, value of '0' */
  CSV_TO_BINARY, /**<calls the csvToBinary() function. '1'. */
  SHOW_DATA,     /**<calls the showData() function. '2'. */
  SEARCH_DATA,   /**<calls the searchData() function. '3'. */
  SEARCH_BY_RRN, /**<calls the searchByRRN() function. '4'. */
} Function;

int main()

{
  int whichFunction;

  if (scanf("%d", &whichFunction) != 1)
    return 0;

  char inputFile[100]; // Simply creates an array to receive the file's name.
  char outputFile[100];

  switch (whichFunction)
  {
    case CSV_TO_BINARY:
    {
      scanf("%s %s", inputFile, outputFile);

      FILE *inputCSVFile = fopen(inputFile, "r");
      FILE *binaryFile = fopen(outputFile, "wb");

      if (inputCSVFile == NULL || binaryFile == NULL) // Error handling
      {
        printf("Falha no processamento do arquivo.\n");

        if (inputCSVFile)
        {
          fclose(inputCSVFile);
        }

        if (binaryFile)
        {
          fclose(binaryFile);
        }
        break; // If the error is caught, close the archives and print error case.
      }

      if (csvToBinary(inputCSVFile, binaryFile) == FUNCTION_SUCESS)
      {
        fclose(inputCSVFile);
        fclose(binaryFile);

        BinarioNaTela(outputFile); // If the function run without problems, it closes the file and print the binary converted from CSV into the screen.
      }
      else
      {
        printf("Falha no processamento do arquivo.\n");
        fclose(inputCSVFile);
        fclose(binaryFile); //Else, signals error and close the files.
      }
      break;
    }

    case SHOW_DATA:
    case SEARCH_DATA:
    case SEARCH_BY_RRN: //Grouped together for the reason of same inputs.
    {

      scanf("%s", inputFile);

      FILE *binaryFile = fopen(inputFile, "rb"); //creates the FILE variable and inserts the designated file onto it.

      if (binaryFile == NULL)
      {
        printf("Falha no processamento do arquivo.\n");
        break;
      } //Error handling

      char status;
      fread(&status, sizeof(char), 1, binaryFile); //Reads the 'status' from the header to check if consistent.

      if (status == STATUS_INCONSISTENT) // Inconsistent = error handling.
      {
        printf("Falha no processamento do arquivo.\n");
        fclose(binaryFile);
        break;
      }

      if (whichFunction == SHOW_DATA) // If the function chosen is showData(), runs it.
      {
        showData(binaryFile);
      }

      else if (whichFunction == SEARCH_DATA) // If the function is searchData(), take the n researches and runs it.
      {
        int n;
        scanf("%d", &n);
        searchData(binaryFile, n);
      }
      else if (whichFunction == SEARCH_BY_RRN) // If its searchByRRN(), get the RRN value and runs it.
      {
        int rrn;
        scanf("%d", &rrn);
        searchByRRN(binaryFile, rrn);
      }

      fclose(binaryFile); //In the end, regardless of input, closes all the files as expected.
      break;
    }
    default:
      printf("Operação inválida.\n"); // Default is responsible for error handling.
      break;
  }

  return 0;
}
