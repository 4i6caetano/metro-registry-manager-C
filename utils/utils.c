#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"
#include "index.h"
#include "registry.h"
#include "functions.h"
#include "header.h"

/* BinarioNaTela é a função fornecida pela disciplina (obrigatória pelo
 * enunciado); sua implementação oficial está em fornecidas.c, compilado
 * junto pelo makefile.
 *
 * scan_quote_string é apenas opcional pelo enunciado ("pode-se usar"),
 * então mantemos aqui a nossa própria implementação: ela para a leitura
 * de um valor sem aspas em espaço/'\r'/'\n' (em vez de pular a quebra de
 * linha como a oficial faz), evitando concatenar com o próximo token
 * quando o valor tem um único caractere (ex.: codEstacao "1"). */
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
    while (binaryToRegistry(&temporaryRegister, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS)
    {
        if (temporaryRegister.removido == IS_NOT_REMOVED &&
            isTheRegistryCorrespondent(&temporaryRegister, fieldsToBeSearched, numberOfFiltersApplied))
        {
            printRegistry(&temporaryRegister);
            (*registersThatFulfillTheSearch)++;
        }

        freeRegistry(&temporaryRegister);
        /* After gathering the fields we must filter, we start this specific search*/
    } // while loop
}

void singleSearchInRegister(Registry temporaryRegister, FILE *registryBinaryFile, Field *fieldsToBeSearched, int numberOfFiltersApplied, int *registersThatFulfillTheSearch)
{
    if (binaryToRegistry(&temporaryRegister, registryBinaryFile) == BINARY_TO_REGISTRY_SUCESS)
    {
        if (temporaryRegister.removido == IS_NOT_REMOVED &&
            isTheRegistryCorrespondent(&temporaryRegister, fieldsToBeSearched, numberOfFiltersApplied))
        {
            printRegistry(&temporaryRegister);
            (*registersThatFulfillTheSearch)++;
        }

        freeRegistry(&temporaryRegister);
        /* After gathering the fields we must filter, we start this specific search*/
    } // while loop
}
