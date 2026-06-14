#ifndef FUNCTIONSINDEX_H
#define FUNCTIONSINDEX_H

#include <stdio.h>

int createPrimaryIndexArchiveInBinary( FILE *registryBinaryFile, FILE *primaryIndexArchive );

int searchOnIndexArchive( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches );

/**
 * @brief Remove logicamente registros que satisfaçam o critério de busca informado,
 * usando a pilha dinâmica de reaproveitamento (campos topo/proximo).
 * A busca segue as mesmas regras de [6]: indexada se codEstacao estiver nos critérios,
 * sequencial caso contrário. Atualiza o arquivo de índice removendo as entradas dos
 * registros removidos. Executa 'numberOfSearches' operações de remoção em sequência.
 *
 * @param registryBinaryFile arquivo binário de dados (leitura e escrita).
 * @param primaryIndexArchive arquivo de índice primário (leitura e escrita).
 * @param numberOfSearches quantidade de operações de remoção a executar.
 * @return FUNCTION_SUCESS em caso de sucesso, FUNCTION_FAILURE em caso de erro.
 */
int removeIndexArchive ( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfSearches );

/**
 * @brief Insere 'numberOfRegistries' novos registros no arquivo de dados,
 * reutilizando espaços de registros logicamente removidos (pilha topo/proximo)
 * quando disponíveis, ou acrescentando ao final quando a pilha estiver vazia.
 * Insere a entrada correspondente no arquivo de índice em ordem crescente por codEstacao.
 *
 * @param registryBinaryFile arquivo binário de dados (leitura e escrita).
 * @param primaryIndexArchive arquivo de índice primário (leitura e escrita).
 * @param numberOfRegistries quantidade de registros a inserir.
 * @return FUNCTION_SUCESS em caso de sucesso, FUNCTION_FAILURE em caso de erro.
 */
int insertNewIndexArchive ( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfRegistries );

/**
 * @brief Atualiza registros do arquivo de dados que satisfaçam o critério de busca,
 * modificando os campos indicados diretamente no registro de tamanho fixo (in-place).
 * A busca segue as mesmas regras de [6]: indexada se codEstacao estiver nos critérios,
 * sequencial caso contrário. Se o codEstacao for alterado, atualiza a entrada no índice.
 * Executa 'numberOfUpdates' operações de atualização em sequência.
 *
 * @param registryBinaryFile arquivo binário de dados (leitura e escrita).
 * @param primaryIndexArchive arquivo de índice primário (leitura e escrita).
 * @param numberOfUpdates quantidade de operações de atualização a executar.
 * @return FUNCTION_SUCESS em caso de sucesso, FUNCTION_FAILURE em caso de erro.
 */
int updateIndexArchive( FILE *registryBinaryFile, FILE *primaryIndexArchive, int numberOfUpdates );

#endif
