#ifndef HEADER_H
#define HEADER_H

#define HEADER_SIZE 17
#define STATUS_INCONSISTENT '1'
#define STATUS_CONSISTENT '0'

typedef struct header {
  char status;
  int topo;
  int proxRRN;
  int nroEstacoes;
  int nroParesEstacao;
} Header;

#endif