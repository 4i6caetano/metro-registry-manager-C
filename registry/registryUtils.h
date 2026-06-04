#ifndef REGISTRYUTILS_H
#define REGISTRYUTILS_H

/**
 * @brief The files are predetermined in order. In consequence of that, this function fills continuously the registry using their order and size to  place them on the proper field.
 * It uses the function getToken() sequentially as an util.
 * @param buffer The buffer (filled up by getToken).
 * @param newRegistry the Registry type variable that will obtain the data.
 */
void fillRegistry(char *buffer, Registry *newRegistry);

/**
 * @brief 
 * Converts the fields of a given registry into a readable binary file.
 * It uses the method of the fwrite() sequentially to write in order, using their sizes as guidance. Adress variable sized fields too.
 * The rest of the archive is converted into garbage '$'.
 * @param newRegistry Registry type variable.
 * @param outputBinaryFile .bin file that will be created / written on.
 */
void registryToBinary(Registry *newRegistry, FILE *outputBinaryFile);

/**
 * @brief binaryToRegistry() has the role of using a given BINARY file, read its contents, and allocates the data on RAM accordingly.
 *
 * It used the fread() sequentially to read the information from the FILE and save it on the right fields.
 *
 * Garbage and void fields are taken into account.
 *
 * @param newRegistry Registry type variable.
 * @param binaryFile binary file that will be read on.
 */
int binaryToRegistry(Registry *newRegistry, FILE *binaryFile);

/**
 * Prints into the terminal the whole Registry information.
 * First, we verify if the data exists. If it does, we write extracting the information directly from the Registry adress using printf().
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