#include<stdio.h>
#include<stdlib.h>
#include<string.h>


#include "header.h"
#include "registry.h"
#include "utils.h"
#include "functionalities.h"

typedef enum function {
  _,
  CSV_TO_BINARY,
  SHOW_DATA,
  SEARCH_DATA,
  SEARCH_BY_RRN,
} Function;

int main()

{
  int whichFunction;

  if(scanf("%d", &whichFunction) != 1) return 0;

  char inputFile[100];
  char outputFile[100];

  switch (whichFunction) {
    case CSV_TO_BINARY: {
      scanf("%s %s", inputFile, outputFile);

      FILE* inputCSVFile = fopen(inputFile, "r");
      FILE* binaryFile = fopen(outputFile, "wb");

      if(inputCSVFile == NULL || binaryFile == NULL){
        printf("Falha no processamento do arquivo.\n");

        if (inputCSVFile){
          fclose(inputCSVFile);
        }

        if(binaryFile){
          fclose(binaryFile);
        }
      }

      if(csvToBinary(inputCSVFile, binaryFile) == FUNCTION_SUCESS){
        fclose(inputCSVFile);
        fclose(binaryFile);

        binarioNaTela(binaryFile);
      }
      else{
        printf("Falha no processamento do arquivo.\n");
        fclose(inputCSVFile);
        fclose(binaryFile);
      }
      break;
    }

    case SHOW_DATA:
    case SEARCH_DATA:
    case SEARCH_BY_RRN: {

      scanf("%s", inputFile);

      FILE* binaryFile = fopen(inputFile, "rb");
      if(binaryFile == NULL){
        printf("Falha no profcessamento do arquivo.\n");
        break;
      }

      char status;
      fread(&status, sizeof(char), 1, binaryFile);

      if(status = STATUS_INCONSISTENT){
        printf("Falha no processamento do arquivo.\n");
        fclose(binaryFile);
        break;
      }

      if(whichFunction == SHOW_DATA){
        showData(binaryFile);
      }

      else if(whichFunction == SEARCH_DATA){
        int n;
        scanf("%d", &n);
        searchData(binaryFile, n);
      }
      else if(whichFunction == SEARCH_BY_RRN){
        int rrn;
        scanf("%d", &rrn);
        searchByRRN(binaryFile, rrn);
      }

      fclose(binaryFile);
      break;
    }
    default:
      printf("Operação inválida.\n");
      break;

  }

  return 0;
}
