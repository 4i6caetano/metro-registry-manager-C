#ifndef FUNCTIONALITIES_C
#define FUNCTIONALITIES_C

#include "functionalities.h"


/**
 *@brief csvToMemory() is the first main function of the project. Utilizing the functions 'fillRegistry' and 'registryToBinary', it reads the .csv file contents into a buffer, transform it into a organized data structure of type Registry, and using this, converts all this data into a .bin file, ultimately, it prints the binary text created.
 * @param inputCSVFile .csv file containing the data, divided by its appropriate fields.
 */
int csvToBinary(FILE* inputCSVFile, FILE* binaryFile){
  char buffer[8000];
  Registry newRegistry;

  if(inputCSVFile == NULL || binaryFile == NULL){
    return FUNCTION_FAILURE;
  }

  writeInitialHeader(binaryFile);
  int rrnCounter = 0;

  char** uniqueStations = malloc(5000 * sizeof(char*));
  int numUniqueStations = 0;

  Pair* uniquePairs = malloc(5000* sizeof(Pair));
  int numUniquePairs = 0;

  fgets(buffer, sizeof(buffer), inputCSVFile);

  while (fgets(buffer, sizeof(buffer), inputCSVFile) != NULL){
      fillRegistry(buffer, &newRegistry);

      if (newRegistry.tamNomeEstacao > 0) {
          addUniqueStation(uniqueStations, &numUniqueStations, newRegistry.nomeEstacao);
      }
      addUniquePair(uniquePairs, &numUniquePairs, newRegistry.codEstacao, newRegistry.codProxEstacao);

      registryToBinary(&newRegistry, binaryFile);
      rrnCounter++;
  }

  updateFinalHeader(binaryFile, rrnCounter, numUniqueStations, numUniquePairs);

  for(int i=0; i<numUniqueStations; i++) free(uniqueStations[i]);
  free(uniqueStations);
  free(uniquePairs);

  return FUNCTION_SUCESS; //SUCESS
};

/*
-> abrir o arquivo em modo leitura binario
-> checagem de erro

->pular o cabeçalho
-> ler por tamanho fixo os registros até o final de arquivo
-> para tamanho variavel -> ler o tamanho INT e pular essa quantidade
-> se for removido, ignorar.

*/

int showData(FILE* binaryFile){

  Registry newRegistry;
  int validRegistry = 0;

  if(binaryFile == NULL){
    printf("Falha no processamento do arquivo.\n");
    return FUNCTION_FAILURE;
  }

  fseek(binaryFile, HEADER_SIZE, SEEK_SET);

  while(binaryToRegistry(&newRegistry, binaryFile) == BINARY_TO_REGISTRY_SUCESS){
    if(newRegistry.removido == IS_NOT_REMOVED){
      printRegistry(&newRegistry);
      validRegistry++;
    }

    freeRegistry(&newRegistry);

  }

if(validRegistry == 0){
  printf("Registro inexistente.\n");
}

  return FUNCTION_SUCESS; //SUCESS

}


typedef struct field{
      char name[100];
      char value[100];
    } Field;

void searchData(FILE* binaryFile, int n){

  if(binaryFile == NULL){
    printf("Falha no processamento do arquivo.\n");
    return;
  }

  for(int i=0; i<n; i++){
    int foundAtleastOne = 0;
    int m = 0;
    scanf("%d", &m);

    Field field[m];

    for(int j=0; j<m; j++){
      scanf("%s", field[j].name);
      ScanQuoteString(field[j].value);
    }

    //-> COMEÇAR A LER O ARQUIVO BINARIO, E ENCONTRAR OS QUE DÃO CERTO
    //-> pra começar a ler, é necessario pular o cabeçalho, e ler sequencialmente até o fim do arquivo. fseek e registryToBinary
    fseek(binaryFile, HEADER_SIZE, SEEK_SET);
    Registry registry;

    while(binaryToRegistry(&registry, binaryFile) == BINARY_TO_REGISTRY_SUCESS){

      if(registry.removido == IS_NOT_REMOVED){ //if the registry is not removed
      int isEqual = 1; //1 if positive, 0 if NOT positive
     //we search in it
        for(int k=0; k<m; k++){

          if(strcmp(field[k].name, "codEstacao") == 0 ){

            int searchValue = (strcmp(field[k].value, "") == 0) ? -1 : atoi(field[k].value);

            if(registry.codEstacao != searchValue){
              isEqual = 0;
              break;
            }

          }

          else if(strcmp(field[k].name, "codLinha") == 0)
          {

            int searchValue = (strcmp(field[k].value, "") == 0) ? -1 : atoi(field[k].value);

            if(registry.codLinha != searchValue){
              isEqual = 0;
              break;
            }
          }

          else if(strcmp(field[k].name, "codProxEstacao") == 0)
          {
            int searchValue = (strcmp(field[k].value, "") == 0) ? -1 : atoi(field[k].value);

            if(registry.codProxEstacao != searchValue){
              isEqual = 0;
              break;
            }
        }

          else if(strcmp(field[k].name, "distProxEstacao") == 0)
          {
            int searchValue = (strcmp(field[k].value, "") == 0) ? -1 : atoi(field[k].value);

            if(registry.distProxEstacao != searchValue){
              isEqual = 0;
              break;
            }

        }

          else if(strcmp(field[k].name, "codLinhaIntegra") == 0)
          {
            int searchValue = (strcmp(field[k].value, "") == 0) ? -1 : atoi(field[k].value);

            if(registry.codLinhaIntegra != searchValue){
              isEqual = 0;
              break;
            }
        }

          else if(strcmp(field[k].name, "codEstIntegra") == 0)
          {
            int searchValue = (strcmp(field[k].value, "") == 0) ? -1 : atoi(field[k].value);
            if(registry.codEstIntegra != searchValue){
              isEqual = 0;
              break;
            }
          }

         else if(strcmp(field[k].name, "nomeEstacao") == 0)
          {
            if(registry.tamNomeEstacao == 0){
              if(strcmp(field[k].value, "") != 0){
                isEqual = 0;
                break;
              }
            } else{
                if(strcmp(registry.nomeEstacao, field[k].value) != 0){
                  isEqual = 0; 
                  break;
                }
              }
          }

          else if (strcmp(field[k].name, "nomeLinha") == 0) {
              if (registry.tamNomeLinha == 0) {
                  if (strcmp(field[k].value, "") != 0) {
                      isEqual = 0; break;
                  }
              } else {
                  if (strcmp(registry.nomeLinha, field[k].value) != 0) {
                      isEqual = 0; 
                      break;
                  }
              }
          }
          

        } //closes the 'k' loop

        if(isEqual == 1){
            printRegistry(&registry);
            foundAtleastOne++;
          }

    } //closes the registry verifier 

    freeRegistry(&registry);

  } // closes the binaryToRegister function

  if(foundAtleastOne == 0){
    printf("Registro inexistente.\n");
    }

  if(i < n-1){
    printf("\n");
  }

  } //closes the 'i' loop.
} //closes the searchData function.

int searchByRRN(FILE* binaryFile, int RRN){
  if(binaryFile == NULL){
    printf("Falha no processamento do arquivo.\n");
    return FUNCTION_FAILURE;
  }

  long offset = HEADER_SIZE + (RRN * 80);

  fseek(binaryFile, offset, SEEK_SET);

  Registry registry;
  
  if(binaryToRegistry(&registry, binaryFile) == BINARY_TO_REGISTRY_SUCESS){

      if(registry.removido == IS_NOT_REMOVED){
        printRegistry(&registry);
        freeRegistry(&registry);
        return FUNCTION_SUCESS;
      } 
      else {
        printf("Registro inexistente.\n");
        freeRegistry(&registry);
        return FUNCTION_FAILURE;
      }
    }

  else{
    printf("Registro inexistente.\n");
    return FUNCTION_FAILURE;
  }

}

#endif