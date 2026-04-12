#ifndef REGISTRY_H
#define REGISTRY_H

#define IS_REMOVED '1'
#define IS_NOT_REMOVED '0'

#define FOUND_RRN '0'
#define DID_NOT_FIND_RRN '1'

#define REGISTRY_SIZE 80

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief The type Registry struct.
 * 
 **char removido;

 ** int proximo; 

 *int codEstacao; 

 *int codLinha;

 * int codProxEstacao;

 * int distProxEstacao;

 * int codLinhaIntegra;

 *  int codEstIntegra; 

 * int tamNomeEstacao;

 * int tamNomeLinha; 

 * char* nomeLinha;
 */
typedef struct registry {

  char removido; /**< (CHAR) '0' NOT removed, '1' REMOVED */
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