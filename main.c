#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"
#include "registry.h"
#include "utils.h"
#include "functions.h"
#include "functionsIndex.h"
#include "index.h"

/**
 * Developers:
 * João Pedro Correia Caetano, NUSP 16987067
 * André Luiz Pereira, NUSP 17068199
 * This code, as well as the process of its development, can be acessed through its github repository:
 * https://github.com/4i6caetano/metro-registry-manager-C
 * Feel free to check it out!
 */

typedef enum function
{
  _,                        /**< campo vazio, valor '0' */
  CSV_TO_BINARY,            /**< chama csvToBinary(). '1'. */
  DISPLAY_VALID_RECORDS,    /**< chama displayValidRecords(). '2'. */
  SEARCH_DATA,              /**< chama searchData(). '3'. */
  SEARCH_BY_RRN,            /**< chama searchByRRN(). '4'. */
  CREATE_PRIMARY_INDEX_ARCHIVE_IN_BINARY,     /**< chama createPrimaryIndexArchive(). '5'. */
  SEARCH_ON_INDEX_ARCHIVE,        /**< chama restoreIndexArchive(). '6'. */
  REMOVE_INDEX_ARCHIVE,        /**< chama removeIndexArchive(). '7'. */
  INSERT_NEW_INDEX_ARCHIVE,        /**< chama insertNewIndexArchive(). '8'. */
  UPDATE_INDEX_ARCHIVE,        /**< chama updateIndexArchive(). '9'. */
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

    case DISPLAY_VALID_RECORDS:
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

      if (whichFunction == DISPLAY_VALID_RECORDS) // If the function chosen is displayValidRecords(), runs it.
      {
        displayValidRecords(binaryFile);
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
    case CREATE_PRIMARY_INDEX_ARCHIVE_IN_BINARY:
    {
      /* [5] Entrada: arquivoEntrada.bin arquivoIndicePrimario.bin */
      char indexFile[100];
      scanf("%s %s", inputFile, indexFile);

      FILE *binaryFile   = fopen(inputFile, "rb");
      FILE *indexBinFile = fopen(indexFile, "wb"); /* cria o arquivo de índice do zero */

      if (binaryFile == NULL || indexBinFile == NULL)
      {
        printf("Falha no processamento do arquivo.\n");
        if (binaryFile)   fclose(binaryFile);
        if (indexBinFile) fclose(indexBinFile);
        break;
      }

      /* verifica se o arquivo de dados está consistente antes de indexar */
      char status;
      fread(&status, sizeof(char), 1, binaryFile);
      if (status == STATUS_INCONSISTENT)
      {
        printf("Falha no processamento do arquivo.\n");
        fclose(binaryFile);
        fclose(indexBinFile);
        break;
      }
      fseek(binaryFile, 0, SEEK_SET);

      if (createPrimaryIndexArchiveInBinary(binaryFile, indexBinFile) == FUNCTION_SUCESS)
      {
        fclose(binaryFile);
        fclose(indexBinFile);
        BinarioNaTela(indexFile);
      }
      else
      {
        fclose(binaryFile);
        fclose(indexBinFile);
      }
      break;
    }

    case SEARCH_ON_INDEX_ARCHIVE:
    {
      /* [6] Entrada: arquivoEntrada.bin arquivoIndicePrimario.bin n */
      char indexFile[100];
      int n;
      scanf("%s %s %d", inputFile, indexFile, &n);

      FILE *binaryFile   = fopen(inputFile, "rb");
      FILE *indexBinFile = fopen(indexFile, "rb");

      if (binaryFile == NULL || indexBinFile == NULL)
      {
        printf("Falha no processamento do arquivo.\n");
        if (binaryFile)   fclose(binaryFile);
        if (indexBinFile) fclose(indexBinFile);
        break;
      }

      /* verifica o status do arquivo de dados */
      char statusDados;
      fread(&statusDados, sizeof(char), 1, binaryFile);
      if (statusDados == STATUS_INCONSISTENT)
      {
        printf("Falha no processamento do arquivo.\n");
        fclose(binaryFile);
        fclose(indexBinFile);
        break;
      }
      fseek(binaryFile, 0, SEEK_SET);

      /* verifica o status do arquivo de índice */
      char statusIndice;
      fread(&statusIndice, sizeof(char), 1, indexBinFile);
      if (statusIndice == INDEX_INCONSISTENT)
      {
        printf("Falha no processamento do arquivo.\n");
        fclose(binaryFile);
        fclose(indexBinFile);
        break;
      }
      fseek(indexBinFile, 0, SEEK_SET);

      searchOnIndexArchive(binaryFile, indexBinFile, n);
      fclose(binaryFile);
      fclose(indexBinFile);
      break;
    }

    case REMOVE_INDEX_ARCHIVE:
    {
      /* [7] Entrada: arquivoEntrada.bin arquivoIndicePrimario.bin n */
      char indexFile[100];
      int n;
      scanf("%s %s %d", inputFile, indexFile, &n);

      FILE *binaryFile   = fopen(inputFile, "r+b");
      FILE *indexBinFile = fopen(indexFile, "r+b");

      if (binaryFile == NULL || indexBinFile == NULL)
      {
        printf("Falha no processamento do arquivo.\n");
        if (binaryFile)   fclose(binaryFile);
        if (indexBinFile) fclose(indexBinFile);
        break;
      }

      /* verifica o status do arquivo de dados */
      char statusDados;
      fread(&statusDados, sizeof(char), 1, binaryFile);
      if (statusDados == STATUS_INCONSISTENT)
      {
        printf("Falha no processamento do arquivo.\n");
        fclose(binaryFile);
        fclose(indexBinFile);
        break;
      }
      fseek(binaryFile, 0, SEEK_SET);

      /* verifica o status do arquivo de índice */
      char statusIndice;
      fread(&statusIndice, sizeof(char), 1, indexBinFile);
      if (statusIndice == INDEX_INCONSISTENT)
      {
        printf("Falha no processamento do arquivo.\n");
        fclose(binaryFile);
        fclose(indexBinFile);
        break;
      }
      fseek(indexBinFile, 0, SEEK_SET);

      if (removeIndexArchive(binaryFile, indexBinFile, n) == FUNCTION_SUCESS)
      {
        fclose(binaryFile);
        fclose(indexBinFile);
        BinarioNaTela(inputFile);
        BinarioNaTela(indexFile);
      }
      else
      {
        fclose(binaryFile);
        fclose(indexBinFile);
      }
      break;
    }

    case INSERT_NEW_INDEX_ARCHIVE :
    {
      /* [8] Entrada: arquivoEntrada.bin arquivoIndicePrimario.bin n */
      char indexFile[100];
      int n;
      scanf("%s %s %d", inputFile, indexFile, &n);

      FILE *binaryFile   = fopen(inputFile, "r+b");
      FILE *indexBinFile = fopen(indexFile, "r+b");

      if (binaryFile == NULL || indexBinFile == NULL)
      {
        printf("Falha no processamento do arquivo.\n");
        if (binaryFile)   fclose(binaryFile);
        if (indexBinFile) fclose(indexBinFile);
        break;
      }

      /* verifica o status do arquivo de dados */
      char statusDados;
      fread(&statusDados, sizeof(char), 1, binaryFile);
      if (statusDados == STATUS_INCONSISTENT)
      {
        printf("Falha no processamento do arquivo.\n");
        fclose(binaryFile);
        fclose(indexBinFile);
        break;
      }
      fseek(binaryFile, 0, SEEK_SET);

      /* verifica o status do arquivo de índice */
      char statusIndice;
      fread(&statusIndice, sizeof(char), 1, indexBinFile);
      if (statusIndice == INDEX_INCONSISTENT)
      {
        printf("Falha no processamento do arquivo.\n");
        fclose(binaryFile);
        fclose(indexBinFile);
        break;
      }
      fseek(indexBinFile, 0, SEEK_SET);

      if (insertNewIndexArchive(binaryFile, indexBinFile, n) == FUNCTION_SUCESS)
      {
        fclose(binaryFile);
        fclose(indexBinFile);
        BinarioNaTela(inputFile);
        BinarioNaTela(indexFile);
      }
      else
      {
        fclose(binaryFile);
        fclose(indexBinFile);
      }
      break;
    }

    case UPDATE_INDEX_ARCHIVE:
    {
      /* [9] Entrada: arquivoEntrada.bin arquivoIndicePrimario.bin n */
      char indexFile[100];
      int n;
      scanf("%s %s %d", inputFile, indexFile, &n);

      FILE *binaryFile   = fopen(inputFile, "r+b");
      FILE *indexBinFile = fopen(indexFile, "r+b");

      if (binaryFile == NULL || indexBinFile == NULL)
      {
        printf("Falha no processamento do arquivo.\n");
        if (binaryFile)   fclose(binaryFile);
        if (indexBinFile) fclose(indexBinFile);
        break;
      }

      /* verifica o status do arquivo de dados */
      char statusDados;
      fread(&statusDados, sizeof(char), 1, binaryFile);
      if (statusDados == STATUS_INCONSISTENT)
      {
        printf("Falha no processamento do arquivo.\n");
        fclose(binaryFile);
        fclose(indexBinFile);
        break;
      }
      fseek(binaryFile, 0, SEEK_SET);

      /* verifica o status do arquivo de índice */
      char statusIndice;
      fread(&statusIndice, sizeof(char), 1, indexBinFile);
      if (statusIndice == INDEX_INCONSISTENT)
      {
        printf("Falha no processamento do arquivo.\n");
        fclose(binaryFile);
        fclose(indexBinFile);
        break;
      }
      fseek(indexBinFile, 0, SEEK_SET);

      if (updateIndexArchive(binaryFile, indexBinFile, n) == FUNCTION_SUCESS)
      {
        fclose(binaryFile);
        fclose(indexBinFile);
        BinarioNaTela(inputFile);
        BinarioNaTela(indexFile);
      }
      else
      {
        fclose(binaryFile);
        fclose(indexBinFile);
      }
      break;
    }

    default:
      printf("Operação inválida.\n");
      break;
  }

  return 0;
}
