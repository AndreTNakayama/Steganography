#ifndef TYPES_H
#define TYPES_H

/* User defined types */
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned char* uchar_ptr;

/* Status will be used in fn. return type */
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
