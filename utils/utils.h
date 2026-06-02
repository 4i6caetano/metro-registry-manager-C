#ifndef UTILS_H

/**
 * @brief Reads and print a .bin file in the terminal.
 */
void BinarioNaTela(char *arquivo);

/**
 * @brief
 *	Use essa função para ler um campo string delimitado entre aspas (").
 *	Chame ela na hora que for ler tal campo. Por exemplo:
 *
 *	A entrada está da seguinte forma:
 *		nomeDoCampo "MARIA DA SILVA"
 *
 *	Para ler isso para as strings já alocadas str1 e str2 do seu programa,
 * você faz: scanf("%s", str1); -> Vai salvar nomeDoCampo em str1
 *		scan_quote_string(str2); -> Vai salvar MARIA DA SILVA em str2
 * (sem as aspas)
 *
 */
void ScanQuoteString(char *str);

/**
 * @brief getToken() is used as a substitute with some additions to the function strtok(), as it was made to include the reading of 'null' elements in the .csv file.
 *
 * This function pick up the data from the .csv file and save it into a buffer.
 */
char *getToken(char **buffer);

#endif