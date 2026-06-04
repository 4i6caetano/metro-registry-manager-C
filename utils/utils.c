#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"
#include "index.h"

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
    size_t fl = ftell(fs); //Saves it on 'fl'.

    fseek(fs, 0, SEEK_SET); // Place the 'cursor' on the first byte.
    unsigned char *mb = (unsigned char *)malloc(fl); // Gets the size on 'fl' and requests a memory of that size.
    fread(mb, 1, fl, fs); // pointer to memory where data will be saved, size of each element, how much it has to read (n*1 byte, total size), the target FILE.

    unsigned long cs = 0;
    for (unsigned long i = 0; i < fl; i++) // until its smaller than the FILE size:
    {
        cs += (unsigned long)mb[i]; // adds the binary to cs.
    }

    printf("%lf\n", (cs / (double)100)); // prints it as a double.

    free(mb); // free memory adress
    fclose(fs); // close the FILE
}

void ScanQuoteString(char *str)
{
    char R;

    while ((R = getchar()) != EOF && isspace(R)) // Reads all 'invalid' chars in the buffer. Simply consumes it. When isspace receives something different than this, skips the loop.
        ; // ignorar espaços, \r, \n...

    if (R == 'N' || R == 'n') //if it finds the letter 'N', skips the word and places it with a "", the program's value for a NULO camp.
    { // campo NULO
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
    const Index *valueA = (Index *) a;
    const Index *valueB = (Index *) b;

    return (valueA->codEstacao - valueB->codEstacao);
}

