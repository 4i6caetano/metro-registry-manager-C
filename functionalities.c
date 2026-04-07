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
void csvToMemory(FILE* inputCSVFile){
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

  while(fread(&newRegistry.removido, sizeof(char), 1, binaryFile) == 1){
    
    fread(&newRegistry.proximo, sizeof(int), 1, binaryFile);
    fread(&newRegistry.codEstacao, sizeof(int), 1, binaryFile);
    fread(&newRegistry.codLinha, sizeof(int), 1, binaryFile);
    fread(&newRegistry.codProxEstacao, sizeof(int), 1, binaryFile);
    fread(&newRegistry.distProxEstacao, sizeof(int), 1, binaryFile);
    fread(&newRegistry.codLinhaIntegra, sizeof(int), 1, binaryFile);
    fread(&newRegistry.codEstIntegra, sizeof(int), 1, binaryFile);
    fread(&newRegistry.tamNomeEstacao, sizeof(int), 1, binaryFile);
    
    if(newRegistry.tamNomeEstacao > 0){
      newRegistry.nomeEstacao = (char*) malloc(newRegistry.tamNomeEstacao + 1);
      fread(newRegistry.nomeEstacao, sizeof(char), newRegistry.tamNomeEstacao, binaryFile);
      newRegistry.nomeEstacao[newRegistry.tamNomeEstacao] = '\0';
    }

    fread(&newRegistry.tamNomeLinha, sizeof(int), 1, binaryFile);
    if (newRegistry.tamNomeLinha > 0) {
      newRegistry.nomeLinha = (char*) malloc(newRegistry.tamNomeLinha + 1);
      fread(newRegistry.nomeLinha, sizeof(char), newRegistry.tamNomeLinha, binaryFile);
      newRegistry.nomeLinha[newRegistry.tamNomeLinha] = '\0';
      }

      if(newRegistry.removido == '0'){
        validRegistry++;
      

        if (newRegistry.codEstacao == -1) printf("NULO "); 
        else printf("%d ", newRegistry.codEstacao);

        if (newRegistry.tamNomeEstacao == 0) printf("NULO "); 
        else printf("%s ", newRegistry.nomeEstacao);

        if (newRegistry.codLinha == -1) printf("NULO "); 
        else printf("%d ", newRegistry.codLinha);

        if (newRegistry.tamNomeLinha == 0) printf("NULO "); 
        else printf("%s ", newRegistry.nomeLinha);

        if (newRegistry.codProxEstacao == -1) printf("NULO "); 
        else printf("%d ", newRegistry.codProxEstacao);

        if (newRegistry.distProxEstacao == -1) printf("NULO "); 
        else printf("%d ", newRegistry.distProxEstacao);

        if (newRegistry.codLinhaIntegra == -1) printf("NULO "); 
        else printf("%d ", newRegistry.codLinhaIntegra);

        if (newRegistry.codEstIntegra == -1) printf("NULO\n"); 
        else printf("%d\n", newRegistry.codEstIntegra);
}

  if(newRegistry.tamNomeEstacao > 0 ){
    free(newRegistry.nomeEstacao);  
  }
  if(newRegistry.tamNomeLinha > 0){
    free(newRegistry.nomeLinha);
  }
}

if(validRegistry == 0){
  printf("Registro inexistente.\n");
}

fclose(binaryFile);


}

#endif