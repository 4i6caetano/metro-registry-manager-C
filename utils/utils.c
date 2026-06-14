#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"
#include "index.h"
#include "registry.h"
#include "functions.h"
#include "header.h"

void BinarioNaTela(char *arquivo)
{
    FILE *fs;
    if (arquivo == NULL || !(fs = fopen(arquivo, "rb"))) // If the file's name is invalid or it cannot open it, does error handling.
    {
        fprintf(stderr,
                "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): "
                "não foi possível abrir o arquivo que me passou para leitura. "
                "Ele existe e você tá passando o nome certo? Você lembrou de "
                "fechar ele com fclose depois de usar?\n");
        return;
    }

    fseek(fs, 0, SEEK_END); // Places the 'cursor' on the last byte of the file.
    size_t fl = ftell(fs);  // Saves it on 'fl'.

    fseek(fs, 0, SEEK_SET);                          // Place the 'cursor' on the first byte.
    unsigned char *mb = (unsigned char *)malloc(fl); // Gets the size on 'fl' and requests a memory of that size.
    fread(mb, 1, fl, fs);                            // pointer to memory where data will be saved, size of each element, how much it has to read (n*1 byte, total size), the target FILE.

    unsigned long cs = 0;
    for (unsigned long i = 0; i < fl; i++) // until its smaller than the FILE size:
    {
        cs += (unsigned long)mb[i]; // adds the binary to cs.
    }

    printf("%lf\n", (cs / (double)100)); // prints it as a double.

    free(mb);   // free memory adress
    fclose(fs); // close the FILE
}

void ScanQuoteString(char *str)
{
    char R;

    while ((R = getchar()) != EOF && isspace(R)) // Reads all 'invalid' chars in the buffer. Simply consumes it. When isspace receives something different than this, skips the loop.
        ;                                        // ignorar espaços, \r, \n...

    if (R == 'N' || R == 'n') // if it finds the letter 'N', skips the word and places it with a "", the program's value for a NULO camp.
    {                         // campo NULO
        getchar();
        getchar();
        getchar();       // ignorar o "ULO" de NULO.
        strcpy(str, ""); // copia string vazia
    }

    else if (R == '\"') // Used for words with spaces on it.
    {
        if (scanf("%[^\"]", str) != 1) // Reads everything, including spaces, until the closing brackets ".
        {
            strcpy(str, ""); // error handling
        }
        getchar(); // ignorar aspas fechando
    }
    else if (R != EOF)
    { // vc tá tentando ler uma string que não tá entre
      // aspas! Fazer leitura normal %s então, pois deve
      // ser algum inteiro ou algo assim...
        str[0] = R;
        str[1] = '\0';
        scanf("%[^ \r\n]", &str[1]);
    }
    else
    { // EOF
        strcpy(str, "");
    }
}

char *getToken(char **buffer)
{
    if (*buffer == NULL) // error handling
        return NULL;

    char *tokenStart = *buffer;               /** char* pointer to the beggining of buffer memory adress, changes throughout the use.  */
    char *tokenEnd = strchr(tokenStart, ','); /** searches for the first ocurrence of a comma, and returns its position. So end - beggining, resulting in the token.  */

    if (tokenEnd != NULL) /** if it still reads values, it places it with a string terminator, and continue tto the next comma occurence. */
    {
        *tokenEnd = '\0';
        *buffer = tokenEnd + 1;
    }
    else // If does not found more commas, simply makes it NULL.
    {
        *buffer = NULL;
    }

    return tokenStart; /** As this is a pointer, it changes the characters directly into the buffer's memory. So, we return the word, and then tokenStart inititates onto the next word. */
}

int compareCodEstacao(const void *a, const void *b)
{
    const Index *valueA = (Index *)a;
    const Index *valueB = (Index *)b;

    return (valueA->codEstacao - valueB->codEstacao);
}

int binarySearchOnIndex(Index *indexArray, int size, int targetKey)
{
    int start = 0;
    int end = size - 1;

    while (start <= end)
    {
        int middle = start + (end - start) / 2;

        // Se encontramos o código da estação no índice
        if (indexArray[middle].codEstacao == targetKey)
        {
            return indexArray[middle].RRN; // Retorna o prêmio: o RRN!
        }

        // Se o código procurado for maior, descarta a metade esquerda
        if (indexArray[middle].codEstacao < targetKey)
        {
            start = middle + 1;
        }
        // Se o código procurado for menor, descarta a metade direita
        else
        {
            end = middle - 1;
        }
    }

    return -1; // Retorna -1 caso o codEstacao não exista no índice
}

void sequentialSearchInRegister(Registry temporaryRegister, FILE *registryBinaryFile, int numberOfFiltersApplied, Field *fieldsToBeSearched, int *registersThatFulfillTheSearch)
{

    int searchSucess;

    while (binaryToRegistry(&temporaryRegister, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS)
    {
        if(temporaryRegister.removido == IS_NOT_REMOVED)
        {
            searchSucess = SEARCH_SUCESS;
            for (int searchForEachFilter = 0; searchForEachFilter < numberOfFiltersApplied; searchForEachFilter++)
            {


                if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "proximo") == 0)
                { // if true, it means this is the filter we want! now we compare its values with each register value
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    if (temporaryRegister.proximo != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "codLinha") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    if (temporaryRegister.codLinha != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "codProxEstacao") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    if (temporaryRegister.codProxEstacao != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "distProxEstacao") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    if (temporaryRegister.distProxEstacao != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "codLinhaIntegra") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    if (temporaryRegister.codLinhaIntegra != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "codEstIntegra") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);

                    if (temporaryRegister.codEstIntegra != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "nomeEstacao") == 0)
                {
                    if (temporaryRegister.tamNomeEstacao == 0)
                    {
                        if (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") != 0)
                        {
                            searchSucess = SEARCH_FAILURE;
                            break;
                        }
                    }
                    else
                    {
                        if (strcmp(temporaryRegister.nomeEstacao, fieldsToBeSearched[searchForEachFilter].valueOfTheField) != 0)
                        {
                            searchSucess = SEARCH_FAILURE;
                            break;
                        }
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "nomeLinha") == 0)
                {
                    if (temporaryRegister.tamNomeLinha == 0)
                    {
                        if (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") != 0)
                        {
                            searchSucess = SEARCH_FAILURE;
                            break;
                        }
                    }

                    else
                    {
                        if (strcmp(temporaryRegister.nomeLinha, fieldsToBeSearched[searchForEachFilter].valueOfTheField) != 0)
                        {
                            searchSucess = SEARCH_FAILURE;
                            break;
                        }
                    }
                }
                
            } // filter loop

            if (searchSucess == SEARCH_SUCESS)
            {
                printRegistry(&temporaryRegister);
                (*registersThatFulfillTheSearch)++;
            }
        }

        freeRegistry(&temporaryRegister);
        /* After gathering the fields we must filter, we start this specific search*/
    } // while loop
}

void singleSearchInRegister(Registry temporaryRegister, FILE *registryBinaryFile, Field *fieldsToBeSearched, int numberOfFiltersApplied, int *registersThatFulfillTheSearch)
{
    int searchSucess;

    if (binaryToRegistry(&temporaryRegister, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS)
    {
        if(temporaryRegister.removido == IS_NOT_REMOVED)
        {
            searchSucess = SEARCH_SUCESS;
            for (int searchForEachFilter = 0; searchForEachFilter < numberOfFiltersApplied; searchForEachFilter++)
            {


                if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "proximo") == 0)
                { // if true, it means this is the filter we want! now we compare its values with each register value
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    if (temporaryRegister.proximo != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "codLinha") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    if (temporaryRegister.codLinha != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "codProxEstacao") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    if (temporaryRegister.codProxEstacao != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "distProxEstacao") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    if (temporaryRegister.distProxEstacao != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "codLinhaIntegra") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);
                    if (temporaryRegister.codLinhaIntegra != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "codEstIntegra") == 0)
                {
                    int value = (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") == 0) ? -1 : atoi(fieldsToBeSearched[searchForEachFilter].valueOfTheField);

                    if (temporaryRegister.codEstIntegra != value)
                    {
                        searchSucess = SEARCH_FAILURE;
                        break;
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "nomeEstacao") == 0)
                {
                    if (temporaryRegister.tamNomeEstacao == 0)
                    {
                        if (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") != 0)
                        {
                            searchSucess = SEARCH_FAILURE;
                            break;
                        }
                    }
                    else
                    {
                        if (strcmp(temporaryRegister.nomeEstacao, fieldsToBeSearched[searchForEachFilter].valueOfTheField) != 0)
                        {
                            searchSucess = SEARCH_FAILURE;
                            break;
                        }
                    }
                }

                else if (strcmp(fieldsToBeSearched[searchForEachFilter].nameOfTheField, "nomeLinha") == 0)
                {
                    if (temporaryRegister.tamNomeLinha == 0)
                    {
                        if (strcmp(fieldsToBeSearched[searchForEachFilter].valueOfTheField, "") != 0)
                        {
                            searchSucess = SEARCH_FAILURE;
                            break;
                        }
                    }

                    else
                    {
                        if (strcmp(temporaryRegister.nomeLinha, fieldsToBeSearched[searchForEachFilter].valueOfTheField) != 0)
                        {
                            searchSucess = SEARCH_FAILURE;
                            break;
                        }
                    }
                }
                
            } // filter loop

            if (searchSucess == SEARCH_SUCESS)
            {
                printRegistry(&temporaryRegister);
                (*registersThatFulfillTheSearch)++;
            }

        }

        freeRegistry(&temporaryRegister);
        /* After gathering the fields we must filter, we start this specific search*/
    } // while loop
}
