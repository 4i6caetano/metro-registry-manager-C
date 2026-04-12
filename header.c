#ifndef HEADER_C

#include "header.h"

void writeInitialHeader(FILE* binaryFile){
  fseek(binaryFile, 0, SEEK_SET);

  char status = STATUS_INCONSISTENT;
  int topo = -1;
  int proxRRN = 0;
  int nroEstacoes = 0;
  int nroParesEstacao = 0;

  fwrite(&status, sizeof(char), 1, binaryFile);
  fwrite(&topo, sizeof(int), 1, binaryFile);
  fwrite(&proxRRN, sizeof(int), 1, binaryFile);
  fwrite(&nroEstacoes, sizeof(int), 1, binaryFile);
  fwrite(&nroParesEstacao, sizeof(int), 1, binaryFile);
}

void updateFinalHeader(FILE* binaryFile, int totalRegistry){
  char status = 'IS_CONSISTENT';

  fseek(binaryFile, 0, SEEK_SET);

  fwrite(&status, sizeof(char), 1, binaryFile);

  fseek(binaryFile, sizeof(int), SEEK_CUR);

  fwrite(&totalRegistry, sizeof(int), 1, binaryFile);
}

#endif