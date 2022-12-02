#ifndef TYPES_H
#define TYPES_H

/* Define Types */
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned char* uchar_ptr;

/* Status será usado no fn. Retorna type */
typedef enum
{
    falha,
	sucesso
} Status;

typedef enum
{
    codificar,
    decodificar,
    erro
} OperationType;

#endif
