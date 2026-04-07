#ifndef HEADER_H
#define HEADER_H

#define HEADER_SIZE 17
#define STATUS_INCONSISTENT '1'
#define STATUS_CONSISTENT '0'


typedef struct header {
  char status; /**< (CHAR) indicates if the file is CONSISTENT = '0', or INCONSISTENT = '1'. By default, the value.  */
  int topo; /**< (CHAR) storages the BYTE OFFSET of the last REMOVED registry. '-1' by default. */
  int proxRRN;  /** next avaible RRN. '0' by default. */
  int nroEstacoes; /**< (INT) indicates the quantity of varied stations storaged in the data file. */
  int nroParesEstacao; /**< (INT) indicates the quantity of different PAIRS (codEstacao, codProxEstacao) storaged in the data file. */
} Header;

#endif