#ifndef FUNCTIONALITIES_C
#define FUNCTIONALITIES_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "functionalities.h"

int writeCSVtoBinary(FILE* inputCSVFile, FILE* outputBinaryFile){

  /*
  1. OPEN 'FOPEN()' and READ the .csv file. (OPEN CSV FILE)
  2. save it into a string buffer. (we DO NOT need the structs to be used now). (CREATE BUFFER AND STORE EVERY CHAR INTO IT)
  3. convert it to binary (CONVERT EVERYTHING TO BINARY)
  4. save its contents and generate a .bin file with it (FOPEN())
  */

  FILE* inputCSVFile;

  inputCSVFile = fopen("estacoes.csv", "r");
  if(inputCSVFile == NULL) return 1;



  fclose(inputCSVFile);



};

#endif