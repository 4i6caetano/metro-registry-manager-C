#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 * Você não precisa entender o código dessa função.
 *
 * Use essa função para comparação no run.codes.
 * Lembre-se de ter fechado (fclose) o arquivo anteriormente.
 *
 * Ela vai abrir de novo para leitura e depois fechar
 * (você não vai perder pontos por isso se usar ela).
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
 * scan_quote_string (ScanQuoteString) é opcional segundo o enunciado
 * ("pode-se usar a função scan_quote_string disponibilizada"). Não a usamos
 * aqui: a versão oficial chama scanf("%s", &str[1]) no caso de valor sem
 * aspas, e %s pula \n ao procurar o próximo token — para valores de um
 * único caractere (ex.: codEstacao "1") isso "vaza" para a próxima linha
 * de entrada e concatena com o próximo token, corrompendo a leitura.
 * Usamos nossa própria implementação (em utils/utils.c), que para nesse
 * caso em '\r'/'\n' em vez de pular a quebra de linha.
 */
