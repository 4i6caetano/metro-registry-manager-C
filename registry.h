#ifndef REGISTRY_H
#define REGISTRY_H

#define IS_REMOVED 1
#define IS_NOT_REMOVED -1

#define DATA_REGISTRY_SIZE 80

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief registry struct structure
 */
typedef struct registry {

  char removido; /**< (CHAR) Denotes if registry is logically removed('0' if NOT removed, '1' if removed). */
  int proximo; /**< (INT) storage the RRN of the next removed registry. */

  int codEstacao; /**< (INT) metro station's code. */
  int codLinha; /** (INT) metro's line code. */
  int codProxEstacao; /** (INT) next station's code. */
  int distProxEstacao; /** (INT) next station's distance. */
  int codLinhaIntegra; /**  (INT) station's integration line.*/
  int codEstIntegra; /** (INT) station's that integrates lines. */

  int tamNomeEstacao; /** (INT) station's name size as integer */
  char* nomeEstacao;  /** (CHAR*) station's name variable fields with fixed registry.  */

  int tamNomeLinha; /** (INT) line's name size as integer.*/
  char* nomeLinha; /** (CHAR*) line's name: variable fields with fixed registry.  */

} Registry;




#endif