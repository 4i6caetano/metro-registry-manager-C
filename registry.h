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

#include "utils.h"
#include "header.h"
#include "functionalities.h"

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


/**
 * @brief This function, used along with getToken(). The latter is responsible for continuously reading the .csv file by updating its token, and as it runs through the file, it fills the Registry type variable accordingly, using its fixed size nature.
 * @param buffer The buffer (filled up by getToken).
 * @param newRegistry the Registry type variable that will obtain the data.
 */
void fillRegistry(char* buffer, Registry *newRegistry);


/**
 * @brief registryToBinary() has the role of converting a given Registry type into a ready-to-read .bin file.
 * 
 * It uses the method of the fwrite() function to collect the specific fields of the Registryn type and write it in order, using the fixed size nature of the registries.
 * The header size is also taken into account and adressed.
 * @param newRegistry Registry type variable.
 * @param outputBinaryFile .bin file that will be created / written on.
 */
void registryToBinary(Registry *newRegistry, FILE* outputBinaryFile);


/**
 * @brief binaryToRegistry() has the role of using a given .bin file, read its contents, and allocate the right data in the right fields of the type Register.
 * 
 * It uses the fread() function considerating the fixed size of the registries. Data conversion also happens
 * 
 * NULL fields, header size and some other problems are taken into account.
 * 
 * @param newRegistry Registry type variable.
 * @param binaryFile binary file that will be read on.
 */
int binaryToRegistry(Registry* newRegistry, FILE* binaryFile);

/**
 * printRegistry() has the role of printing on screen a full registry.
 * 
 * It receives a pointer and reads the registry contents. Using booleans, we verify if the data exists and if its '-1' (NULL), writing according to it.
 * @param newRegistry Registry type variable.
 */
void printRegistry(Registry* newRegistry);

/**
 * @brief freeRegistry() simply free the allocated space used in the registries, like their (varied) line and station names.
 * 
 * @param Registry type variable.
 */
void freeRegistry(Registry* newRegistry);


#endif