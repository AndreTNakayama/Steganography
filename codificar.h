#ifndef ENCODE_H
#define ENCODE_H

#include "types.h"
#include "header.h"

// Informação necessaria para codificar e decodificar

#define FILENAME_SIZE   	50
#define FILE_SUFFIX     	4
#define INT_SIZE            sizeof(int)
#define CHAR_SIZE           sizeof(char)

typedef struct _EncodeInfo
{
    FILE *ptf_imagem_origem;
    uchar nome_imagem_origem[FILENAME_SIZE];
    uint tamanho_imagem;
    FILE *ptr_mensagem;
    uchar mensagem_arquivo[FILENAME_SIZE];
    uchar mensagem_extensao[FILE_SUFFIX + CHAR_SIZE];
    uint tamanho_mensagem;
    uint tamanho_magic_string;
    uint tamanho_msg_extensao;
    FILE *ptf_imagem_retorno;
    uchar nome_imagem_retorno[FILENAME_SIZE];
    FILE* ptr_decodificar_arquivo;
    uchar nome_arquivo_decodificar[FILENAME_SIZE];
} EncodeInfo;

/* Verifica o tipo da operação */
OperationType check_operation_type(char *argv[]);

/* Le e valida os argumentos para codificar */
Status read_and_validate_bmp_format(char *argv[]);

/* Executa a codificação */
Status do_encoding(EncodeInfo *encInfo);

/* Executa a decodificação */
Status do_decoding(EncodeInfo *encInfo);

/* Pega os ponteiros dos arquivos */
Status open_files(EncodeInfo *encInfo);

/* Verificar tamanho */
Status check_capacity(EncodeInfo *encInfo);

/* Pegar tamanho da imagem */
uint get_image_size_for_bmp(FILE *fptr_image);

/* Pegar tamanho do arquivo */
uint get_file_size(FILE *fptr);

/* Copiar o cabeçalho da imagem */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);

/* Codifica a magic string */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo);

/* Codifica a extensão do arquivo da mensagem */
Status encode_int_size_expression(uint len, EncodeInfo *encInfo);

/* Copia os bytes restantes do src para a imagem de destino depois de codificar */
Status copy_remaining_image_data(FILE *fptr_src, FILE *fptr_dest, uint f_size);

/* Decodifica a magic string */
uchar_ptr decode_magic_string(uint size, EncodeInfo *encInfo);

/* Decodificar o tamanho */
uint decode_int_size_expression(EncodeInfo *encInfo);

/* Ler, validar e extrair a extensão do arquivo escondido */
Status read_and_validate_extn(uchar_ptr sec_file_name_holder, EncodeInfo *encInfo);

/* Decodificar a mensagem escondida */
Status decode_file_data(uint f_size, EncodeInfo *encInfo);

#endif
