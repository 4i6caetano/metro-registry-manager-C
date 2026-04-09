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

//CodEstacao,NomeEstacao,CodLinha,NomeLinha,CodProxEst,DistanciaProxEst,CodLinhaInteg,CodEstacaoInteg
// escrever o csv no buffer -> pegar o buffer e escrever tudo em registros -> a cada registro, escrever em binario

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

#endif