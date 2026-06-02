#ifndef REGISTRYUTILS_H
#define REGISTRYUTILS_H

/**
 * @brief This function, used along with getToken(). The latter is responsible for continuously reading the .csv file by updating its token, and as it runs through the file, it fills the Registry type variable accordingly, using its fixed size nature.
 * @param buffer The buffer (filled up by getToken).
 * @param newRegistry the Registry type variable that will obtain the data.
 */
void fillRegistry(char *buffer, Registry *newRegistry);

/**
 * @brief registryToBinary() has the role of converting a given Registry type into a ready-to-read .bin file.
 *
 * It uses the method of the fwrite() function to collect the specific fields of the Registryn type and write it in order, using the fixed size nature of the registries.
 * The header size is also taken into account and adressed.
 * @param newRegistry Registry type variable.
 * @param outputBinaryFile .bin file that will be created / written on.
 */
void registryToBinary(Registry *newRegistry, FILE *outputBinaryFile);

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
int binaryToRegistry(Registry *newRegistry, FILE *binaryFile);

/**
 * printRegistry() has the role of printing on screen a full registry.
 *
 * It receives a pointer and reads the registry contents. Using booleans, we verify if the data exists and if its '-1' (NULL), writing according to it.
 * @param newRegistry Registry type variable.
 */
void printRegistry(Registry *newRegistry);

/**
 * @brief freeRegistry() simply free the allocated space used in the registries, like their (varied) line and station names.
 *
 * @param Registry type variable.
 */
void freeRegistry(Registry *newRegistry);

#endif