#ifndef REGISTRYUTILS_H
#define REGISTRYUTILS_H

/**
 * @brief Populates a Registry structure by parsing a comma-separated string buffer.
 *
 * @details This function sequentially extracts tokens from the provided buffer 
 * (which represents a single line from a CSV file) using the `getToken()` utility. 
 * Since the CSV fields follow a predetermined strict order, the function maps each 
 * token to its corresponding field in the `Registry` structure. It properly handles 
 * both fixed-size numeric fields and dynamically allocates memory for variable-size strings.
 *
 * @param buffer A pointer to the string containing the CSV line to be parsed.
 * @param newRegistry Pointer to the Registry structure that will receive the parsed data.
 */
void fillRegistry(char *buffer, Registry *newRegistry);

/**
 * @brief Serializes and writes a populated Registry structure into a binary file.
 *
 * @details This function converts the data stored in RAM into binary format and writes 
 * it to the file using `fwrite()`. It strictly adheres to the project's physical schema: 
 * writing the fixed-size fields, the variable-size strings, and their respective size 
 * indicators. After writing the valid data, it calculates the remaining bytes up to the 
 * fixed 80-byte record size limit and fills that space with garbage characters ('$').
 *
 * @param newRegistry Pointer to the Registry structure containing the data to be written.
 * @param outputBinaryFile Pointer to the binary file (must be open for writing).
 */
void registryToBinary(Registry *newRegistry, FILE *outputBinaryFile);

/**
 * @brief Reads a single record from a binary file and loads it into a Registry structure in RAM.
 *
 * @details This function uses `fread()` to sequentially read the bytes of an 80-byte record 
 * starting from the current file pointer offset. It properly maps the binary data to the 
 * `Registry` fields, dynamically allocating memory for variable-length strings (like station 
 * and line names) based on their read size indicators. It correctly interprets null/empty 
 * fields and seamlessly consumes the '$' garbage padding at the end of the block.
 *
 * @param newRegistry Pointer to the Registry structure where the read data will be stored.
 * @param binaryFile Pointer to the binary file (must be open for reading).
 * @return int Returns a success macro (e.g., BINARY_TO_REGISTRY_SUCESS) if a record was 
 * successfully read, or a failure macro if it encounters the End of File (EOF) or an error.
 */
int binaryToRegistry(Registry *newRegistry, FILE *binaryFile);

/**
 * @brief Formats and prints the contents of a Registry to the standard output.
 *
 * @details This function outputs the data of a single, valid record to the terminal. 
 * It evaluates each field for nullability (i.e., empty strings or integer fields set 
 * to -1). If a field is null, it prints "NULO " instead of the value. It ensures proper 
 * formatting by separating fields with spaces and printing a single newline character (`\n`) 
 * only after the final field (`codEstIntegra`).
 *
 * @param newRegistry Pointer to the Registry structure whose data is to be printed.
 */
void printRegistry(Registry *newRegistry);

/**
 * @brief Frees dynamically allocated memory within a Registry structure.
 *
 * @details Because variable-length fields (`nomeEstacao` and `nomeLinha`) are dynamically 
 * allocated on the heap (e.g., using `malloc` or `strdup`) during reading and parsing 
 * operations, this function safely calls `free()` on these specific pointers. It is 
 * essential to call this function to prevent memory leaks before a `Registry` struct 
 * goes out of scope or is overwritten by a new record.
 *
 * @param newRegistry Pointer to the Registry structure whose internal strings need freeing.
 */
void freeRegistry(Registry *newRegistry);

#endif