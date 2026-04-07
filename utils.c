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

void fillRegistry(char* buffer, Registry *newRegistry){
  char* token;
  token = strtok(buffer, ",");
  newRegistry->codEstacao = atoi(token); 

  token = strtok(NULL, ",");
  newRegistry->tamNomeEstacao = strlen(token);
  newRegistry->nomeEstacao = strdup(token);

  token = strtok(NULL, ",");
  newRegistry->codLinha = atoi(token);

  token = strtok(NULL, ",");
  newRegistry->tamNomeLinha = strlen(token);
  newRegistry->nomeLinha = strdup(token);

  token = strtok(NULL, ",");
  newRegistry->codProxEstacao = atoi(token);
  
  token = strtok(NULL, ",");
  newRegistry->distProxEstacao = atoi(token);

  token = strtok(NULL, ",");
  newRegistry->codLinhaIntegra = atoi(token);

  token = strtok(NULL, ",");
  newRegistry->codEstIntegra = atoi(token);

  newRegistry->removido = '0';
  newRegistry->proximo = -1;
  };

  void registryToBinary(Registry *newRegistry, FILE* outputBinaryFile){

    //Campos de tamanho fixo
    fwrite(&newRegistry->removido, sizeof(char), 1, outputBinaryFile);
    fwrite(&newRegistry->proximo, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->codEstacao, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->codLinha, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->codProxEstacao, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->distProxEstacao, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->codLinhaIntegra, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->codEstIntegra, sizeof(int), 1, outputBinaryFile);
    fwrite(&newRegistry->tamNomeEstacao, sizeof(int), 1, outputBinaryFile);

    //Campos de tamanho variável
    if(newRegistry->tamNomeEstacao > 0){
      fwrite(newRegistry->nomeEstacao, sizeof(char), newRegistry->tamNomeEstacao, outputBinaryFile);
      free(newRegistry->nomeEstacao);
    }
    fwrite(&newRegistry->tamNomeLinha, sizeof(int), 1, outputBinaryFile);
    if(newRegistry->tamNomeLinha > 0){
      fwrite(&newRegistry->nomeLinha, sizeof(char), newRegistry->tamNomeLinha, outputBinaryFile);
    }
  };

int csvToMemory(FILE* inputCSVFile, FILE* outputBinaryFile){
char buffer[8000];
Registry newRegistry;
FILE* outputBinaryFile;
outputBinaryFile = fopen('estacoes.bin', 'ab');

  fgets(buffer, sizeof(buffer), inputCSVFile);

  while (fgets(buffer, sizeof(buffer), inputCSVFile) != NULL){
    fillRegistry(buffer, &newRegistry);
    registryToBinary(&newRegistry, outputBinaryFile);
};
};

//CodEstacao,NomeEstacao,CodLinha,NomeLinha,CodProxEst,DistanciaProxEst,CodLinhaInteg,CodEstacaoInteg
// escrever o csv no buffer -> pegar o buffer e escrever tudo em registros -> a cada registro, escrever em binario

#endif