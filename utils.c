#ifndef UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include "registry.h"

void BinarioNaTela(char *arquivo) {
    FILE *fs;
    if (arquivo == NULL || !(fs = fopen(arquivo, "rb"))) {
        fprintf(stderr,
                "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): "
                "não foi possível abrir o arquivo que me passou para leitura. "
                "Ele existe e você tá passando o nome certo? Você lembrou de "
                "fechar ele com fclose depois de usar?\n");
        return;
    }

    fseek(fs, 0, SEEK_END);
    size_t fl = ftell(fs);

    fseek(fs, 0, SEEK_SET);
    unsigned char *mb = (unsigned char *)malloc(fl);
    fread(mb, 1, fl, fs);

    unsigned long cs = 0;
    for (unsigned long i = 0; i < fl; i++) {
        cs += (unsigned long)mb[i];
    }

    printf("%lf\n", (cs / (double)100));

    free(mb);
    fclose(fs);
}

/*
 *	Use essa função para ler um campo string delimitado entre aspas (").
 *	Chame ela na hora que for ler tal campo. Por exemplo:
 *
 *	A entrada está da seguinte forma:
 *		nomeDoCampo "MARIA DA SILVA"
 *
 *	Para ler isso para as strings já alocadas str1 e str2 do seu programa,
 * você faz: scanf("%s", str1); // Vai salvar nomeDoCampo em str1
 *		scan_quote_string(str2); // Vai salvar MARIA DA SILVA em str2
 * (sem as aspas)
 *
 */
void ScanQuoteString(char *str) {
    char R;

    while ((R = getchar()) != EOF && isspace(R))
        ; // ignorar espaços, \r, \n...

    if (R == 'N' || R == 'n') { // campo NULO
        getchar();
        getchar();
        getchar();       // ignorar o "ULO" de NULO.
        strcpy(str, ""); // copia string vazia
    } else if (R == '\"') {
        if (scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
            strcpy(str, "");
        }
        getchar();         // ignorar aspas fechando
    } else if (R != EOF) { // vc tá tentando ler uma string que não tá entre
                           // aspas! Fazer leitura normal %s então, pois deve
                           // ser algum inteiro ou algo assim...
        str[0] = R;
        scanf("%s", &str[1]);
    } else { // EOF
        strcpy(str, "");
    }
}

char* getToken(char** buffer) {
    if (*buffer == NULL) return NULL;
    
    char* tokenStart = *buffer;
    char* tokenEnd = strchr(tokenStart, ',');
    
    if (tokenEnd != NULL) {
        *tokenEnd = '\0'; 
        *buffer = tokenEnd + 1; 
    } else {
        *buffer = NULL; 
    }
    
    return tokenStart;
}

void fillRegistry(char* buffer, Registry *newRegistry){
  buffer[strcspn(buffer, "\r\n")] = '\0';

  char* rest = buffer;
  char* token;

  token = getToken(&rest);
  newRegistry->codEstacao = atoi(token); 

  token = getToken(&rest);
  newRegistry->tamNomeEstacao = strlen(token);
  newRegistry->nomeEstacao = strdup(token);

  token = getToken(&rest);
  newRegistry->codLinha = (strlen(token) == 0) ? -1 : atoi(token);

  token = getToken(&rest);
  newRegistry->tamNomeLinha = strlen(token);
  newRegistry->nomeLinha = ((newRegistry->tamNomeLinha) == 0) ? NULL : strdup(token);

  token = getToken(&rest);
  newRegistry->codProxEstacao = (strlen(token) == 0) ? -1 : atoi(token);
  
  token = getToken(&rest);
  newRegistry->distProxEstacao = (strlen(token) == 0) ? -1 : atoi(token);

  token = getToken(&rest);
  newRegistry->codLinhaIntegra = (strlen(token) == 0) ? -1 : atoi(token);

  token = getToken(&rest);
  newRegistry->codEstIntegra = (strlen(token) == 0) ? -1 : atoi(token);

  newRegistry->removido = '0';
  newRegistry->proximo = -1;
  };

  void registryToBinary(Registry *newRegistry, FILE* outputBinaryFile){

    //Fixed size fields
    fwrite(&newRegistry->removido, sizeof(char), 1, outputBinaryFile);
    fwrite(&newRegistry->proximo, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->codEstacao, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->codLinha, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->codProxEstacao, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->distProxEstacao, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->codLinhaIntegra, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->codEstIntegra, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->tamNomeEstacao, sizeof(int), 1, outputBinaryFile);

    //Variable size fields
    if(newRegistry->tamNomeEstacao > 0){
      fwrite(newRegistry->nomeEstacao, sizeof(char), newRegistry->tamNomeEstacao, outputBinaryFile);
      free(newRegistry->nomeEstacao);
    }
    fwrite(&newRegistry->tamNomeLinha, sizeof(int), 1, outputBinaryFile);
    if(newRegistry->tamNomeLinha > 0){
      fwrite(newRegistry->nomeLinha, sizeof(char), newRegistry->tamNomeLinha, outputBinaryFile);
      free(newRegistry->nomeLinha);
    }
  };

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

#endif