#ifndef UTILS_H

#include "utils.h"

void binarioNaTela(FILE *arquivo_binario) {
  if (arquivo_binario == NULL) return;

  rewind(arquivo_binario);

  int caracter;
  while((caracter = fgetc(arquivo_binario)) != EOF){
    putchar(caracter);
  }
}

#endif