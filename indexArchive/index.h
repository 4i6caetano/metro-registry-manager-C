#ifndef INDEX_H
#define INDEX_H

#define INDEX_SIZE 8

#define INDEX_CONSISTENT '1'
#define INDEX_INCONSISTENT '0'

typedef struct indexHeader
{
    char status; /**< changed to INCONSISTENT '0' when the file is altered, then CONSISTENT '1' when the process is done. */
} IndexHeader;

typedef struct index 
{
    
    int codEstacao;
    int RRN;

} Index;

#endif