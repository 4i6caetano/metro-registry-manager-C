#ifndef UTILS_H

#include "utils.h"


/**
 * @brief Reads and print a .bin file.
 */
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


/**
 * @brief Substitute function of strtok, read continuously a buffer until the file ends. Handle void fields and parse using ','.
 */
char* getToken(char** buffer) {
    if (*buffer == NULL) return NULL;
    
    char* tokenStart = *buffer; /**char* pointer to the beggining of buffer memory adress, changes throughout the use.  */
    char* tokenEnd = strchr(tokenStart, ','); /**char* pointer responsible for separating the fields, is updated throughout the use.*/
    
    if (tokenEnd != NULL) {
        *tokenEnd = '\0'; 
        *buffer = tokenEnd + 1; 
    } else {
        *buffer = NULL; 
    }
    
    return tokenStart;
}

/**
 * @brief function responsible for transforming the 'string' fields of the .csv file into the Registry struct structure.
 * Uses getToken().
 * 
 * To convert to numbers, 'atoi()' is used.
 * To deal with sizes, 'strlen()' is used.
 * To handle void fields, the ternary operator is implemented so it can fill it with '-1' for INT and '0' for STRING.
 * @param buffer buffer to storage the data.
 * @param newRegistry a Registry type struct 
 */
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


  /**
   * @brief registryToBinary(), using fwrite(), writes all the Registry type fields into binary, using a .bin file given.
   * 
   * Measures were used to ensure the names of the station and lines were avaible.
   * @param newRegistry a given Registry type struct that contain the data in its fields.
   * @param outputBinaryFile a given .bin that will be written on.
   */
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

  int binaryToRegistry(Registry* newRegistry, FILE* binaryFile){

    if(binaryFile == NULL) return;

    if (fread(&newRegistry->removido, sizeof(char), 1, binaryFile) != 1) {
        return BINARY_TO_REGISTRY_FAILURE; 
    }
    
        fread(&newRegistry->proximo, sizeof(int), 1, binaryFile);
        fread(&newRegistry->codEstacao, sizeof(int), 1, binaryFile);
        fread(&newRegistry->codLinha, sizeof(int), 1, binaryFile);
        fread(&newRegistry->codProxEstacao, sizeof(int), 1, binaryFile);
        fread(&newRegistry->distProxEstacao, sizeof(int), 1, binaryFile);
        fread(&newRegistry->codLinhaIntegra, sizeof(int), 1, binaryFile);
        fread(&newRegistry->codEstIntegra, sizeof(int), 1, binaryFile);

        fread(&newRegistry->tamNomeEstacao, sizeof(int), 1, binaryFile);
        
        if(newRegistry->tamNomeEstacao > 0){
        newRegistry->nomeEstacao = (char*) malloc(newRegistry->tamNomeEstacao + 1);
        fread(newRegistry->nomeEstacao, sizeof(char), newRegistry->tamNomeEstacao, binaryFile);
        newRegistry->nomeEstacao[newRegistry->tamNomeEstacao] = '\0';
        }
        else{
            newRegistry->nomeEstacao == NULL;
        }

        fread(&newRegistry->tamNomeLinha, sizeof(int), 1, binaryFile);

        if (newRegistry->tamNomeLinha > 0) {
        newRegistry->nomeLinha = (char*) malloc(newRegistry->tamNomeLinha + 1);
        fread(newRegistry->nomeLinha, sizeof(char), newRegistry->tamNomeLinha, binaryFile);
        newRegistry->nomeLinha[newRegistry->tamNomeLinha] = '\0';
        }
        else{
            newRegistry->nomeLinha == NULL;
        }

        int countBytes = 37 + newRegistry->tamNomeEstacao + newRegistry->tamNomeLinha;
        int memoryGarbage = 80 - countBytes;
        
        if(memoryGarbage > 0){
            fseek(binaryFile, memoryGarbage, SEEK_CUR);
        }

        return BINARY_TO_REGISTRY_SUCESS;
    }

    void printRegistry(Registry* newRegistry){

        if (newRegistry->codEstacao == -1) printf("NULO "); 
        else printf("%d ", newRegistry->codEstacao);

        if (newRegistry->tamNomeEstacao == 0) printf("NULO "); 
        else printf("%s ", newRegistry->nomeEstacao);

        if (newRegistry->codLinha == -1) printf("NULO "); 
        else printf("%d ", newRegistry->codLinha);

        if (newRegistry->tamNomeLinha == 0) printf("NULO "); 
        else printf("%s ", newRegistry->nomeLinha);

        if (newRegistry->codProxEstacao == -1) printf("NULO "); 
        else printf("%d ", newRegistry->codProxEstacao);

        if (newRegistry->distProxEstacao == -1) printf("NULO "); 
        else printf("%d ", newRegistry->distProxEstacao);

        if (newRegistry->codLinhaIntegra == -1) printf("NULO "); 
        else printf("%d ", newRegistry->codLinhaIntegra);

        if (newRegistry->codEstIntegra == -1) printf("NULO\n"); 
        else printf("%d\n", newRegistry->codEstIntegra);
}

void freeRegistry(Registry* newRegistry) {
    if(newRegistry->tamNomeEstacao > 0) {
        free(newRegistry->nomeEstacao);  
    }
    if(newRegistry->tamNomeLinha > 0) {
        free(newRegistry->nomeLinha);
    }
    newRegistry->nomeEstacao = NULL;
    newRegistry->nomeLinha = NULL;
}

#endif