#include "stdio.h"

#include "registry.h"
#include "utils.h"


void contarUnicosValidos(FILE *arq, int *nEst, int *nPares);

//calcula quantas entradas de dados ha no arquivo de indice
//tamanho total menos 1 byte de cabecalho dividido por 8 bytes por entrada
int contarEntradasIndice(FILE *arqIndice);

//remove do arquivo de indice a entrada com o codEstacao informado
//estrategia: carrega tudo na memoria, reescreve pulando a entrada removida,
//depois trunca o arquivo para o novo tamanho (sem isso sobra lixo no final)
void removerDoIndice(FILE *arqIndice, int codEstacao);

//insere uma nova entrada no arquivo de indice mantendo a ordem crescente por codEstacao
//estrategia: carrega tudo na memoria, acha onde inserir, empurra os maiores uma posicao
//para a direita abrindo espaco, depois reescreve o arquivo inteiro com a nova entrada
void inserirNoIndice(FILE *arqIndice, int codEstacao, int rrn);

//aplica os p pares (campo, novo valor) sobre o registro que esta em memoria
//para campos string: libera a string antiga antes de alocar a nova
//valor vazio "" vira -1 para inteiros e NULL para strings (campo nulo)
void aplicarAtualizacoes(Registry *reg, Field *camposAtu, int p);

// 1. Verifica se o codEstacao é um dos filtros aplicados pelo usuário.
int obterCodEstacaoBusca(Field *campos, int m);

// 2. Testa o registro atual contra todos os filtros (usada nas funções 7 e 9 dele)
int registroCorresponde(Registry *reg, Field *campos, int m);

// 3. Lê dados brutos do terminal para inserir um registro novo (Funcionalidade 8)
void lerRegistroStdin(Registry *reg);