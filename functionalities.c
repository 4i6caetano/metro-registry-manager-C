#ifndef FUNCTIONALITIES_C
#define FUNCTIONALITIES_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "functionalities.h"
#include "utils.h"
#include "header.h"

/**
 *@brief csvToMemory() is the first main function of the project. Utilizing the functions 'fillRegistry' and 'registryToBinary', it reads the .csv file contents into a buffer, transform it into a organized data structure of type Registry, and using this, converts all this data into a .bin file, ultimately, it prints the binary text created.
 * @param inputCSVFile .csv file containing the data, divided by its appropriate fields.
 */
void csvToBinary(FILE* inputCSVFile){
  char buffer[8000];
  Registry newRegistry;
  FILE* outputBinaryFile = fopen("estacoes.bin", "wb");

  if(outputBinaryFile == NULL){
    return;
  }

    fgets(buffer, sizeof(buffer), inputCSVFile);

    while (fgets(buffer, sizeof(buffer), inputCSVFile) != NULL){
      fillRegistry(buffer, &newRegistry);
      registryToBinary(&newRegistry, outputBinaryFile);
  }

    fclose(outputBinaryFile);
    BinarioNaTela("estacoes.bin");
};

/*
-> abrir o arquivo em modo leitura binario
-> checagem de erro

->pular o cabeçalho
-> ler por tamanho fixo os registros até o final de arquivo
-> para tamanho variavel -> ler o tamanho INT e pular essa quantidade
-> se for removido, ignorar.

*/

void showData(FILE* binaryFile){

  Registry newRegistry;
  int validRegistry = 0;

  if(binaryFile == NULL){
    printf("Falha no processamento do arquivo.\n");
    return;
  }

  fseek(binaryFile, HEADER_SIZE, SEEK_SET);

  while(binaryToRegistry(&newRegistry, binaryFile) == 1){
    if(newRegistry.removido == '0'){
      printRegistry(&newRegistry);
      validRegistry++;
    }

    freeRegistry(&newRegistry);

  }

if(validRegistry == 0){
  printf("Registro inexistente.\n");
}

fclose(binaryFile);

}

typedef struct field{
      char name[100];
      char value[100];
    } Field;

void searchData(FILE* binaryFile){

  if(binaryFile == NULL){
    printf("Falha no processamento de arquivo.\n");
    return;
  }
  
  int n = 0;
  scanf("%d", &n);

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

    while(binaryToRegistry(&registry, binaryFile) == 1){

      if(registry.removido == '0'){ //if the registry is not removed
      int isEqual = 1; //1 if positive, 0 if NOT positive
     //we search in it
        for(int k=0; k<m; k++){

          if(strcmp(field[k].name, "codEstacao") == 0 ){
            if(registry.codEstacao != atoi(field[k].value)){
              isEqual = 0;
              break;
            }
          }

          else if(strcmp(field[k].name, "codLinha") == 0)
          {
            if(registry.codLinha != atoi(field[k].value)){
              isEqual = 0;
              break;
            }
          }

          else if(strcmp(field[k].name, "codProxEstacao") == 0)
          {
            if(registry.codProxEstacao != atoi(field[k].value)){
              isEqual = 0;
              break;
          }
        }

          else if(strcmp(field[k].name, "distProxEstacao") == 0)
          {
            if(registry.distProxEstacao != atoi(field[k].value)){
              isEqual = 0;
              break;
          }
        }

          else if(strcmp(field[k].name, "codLinhaIntegra") == 0)
          {
            if(registry.codLinhaIntegra != atoi(field[k].value)){
              isEqual = 0;
              break;
          }
        }

          else if(strcmp(field[k].name, "codEstIntegra") == 0)
          {
            if(registry.codEstIntegra != atoi(field[k].value)){
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

//-> binaryToRegistry (transformar cada registro)
//-> E em cada um dos registros, procurar se o nome e valor são iguais.
//Então isso teria que ser dentro do loop j
if(foundAtleastOne == 0){
          printf("Registro inexistente.\n");
        }

} //closes the 'i' loop.
} //closes the searchData function.

#endif