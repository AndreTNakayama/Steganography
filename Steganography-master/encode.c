#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "header.h"

//Extern required values from main file
extern uint raster_data;
extern uchar magic_string_signature[CHAR_SIZE + CHAR_SIZE];

OperationType check_operation_type(char *argv[])
{
	if (!strcmp(*argv, "-c"))//Check if argument is '-e'
		return codificar;
	else if (!strcmp(*argv, "-d"))//Check if argument is '-d'
		return decodificar;
	else
		return erro;//Some other argument
}

/* Function Definitions */
Status copy_bmp_header(FILE *ptf_imagem_origem, FILE *fptr_dest_image)
{
	//Store image header of the size of raster data into a block of heap memory
	uchar_ptr img_header = (uchar_ptr) malloc(raster_data * sizeof(uchar));

	fread(img_header, raster_data, 1, ptf_imagem_origem);

	//Write data obtained in heap onto the destination file
	fwrite(img_header, raster_data, 1, fptr_dest_image);

	free(img_header);//Free dynamically allocated block of memory
	return sucesso;//No error found
}

uint get_image_size_for_bmp(FILE *fptr_image)
{
	uint img_size;//Return value to be read from image file
	//Seek to 34th byte to get image data size from the '.bmp' image file
	fseek(fptr_image, 34L, SEEK_SET);
	//Read the width (an unsigned integer)
	fread(&img_size, sizeof(img_size), 1, fptr_image);
	//Return image capacity
	return img_size;
}

uint get_file_size(FILE *fptr)//Returns file size including EOF byte
{
	//Seek to the end of file
	fseek(fptr, 0L, SEEK_END);
	return (uint) ftell(fptr);//Return file index value
}

Status open_files(EncodeInfo *encInfo)
{
	//Open source image file with proper error handling
	encInfo->ptf_imagem_origem = fopen((const char*)encInfo->nome_imagem_origem, "rb");

	//Open secret file with proper error handling
	encInfo->ptr_mensagem = fopen((const char*)encInfo->mensagem_arquivo, "rb");

	//Open output image file with proper error handling
	encInfo->ptf_imagem_retorno = fopen((const char*)encInfo->nome_imagem_retorno, "wb");

	return sucesso;//No error found
}

Status read_and_validate_bmp_format(char *argv[])
{
	//Pointer to hold address of '.bmp' part from given argument
	const char* bmp_holder = strstr(*argv, ".bmp");
	if(bmp_holder)//Error handling
	{//If '.bmp' part is found, then check if string exactly ends with '.bmp'
			return (!strcmp(bmp_holder, ".bmp")) ? sucesso : falha;
	}
	return falha;//NULL address which means '.bmp' part is not found
}

Status read_and_validate_extn(uchar_ptr sec_file_name_holder, EncodeInfo *encInfo)
{
	//Pointer to hold the heap memory of the size of filename including '\0' character
	uchar_ptr sec = (uchar_ptr) malloc(strlen((const char*)sec_file_name_holder) + 1);

	strcpy((char*)sec, (const char*)sec_file_name_holder);//Store the filename inside allocated heap
	uint secret_filename_len = strlen((const char*)sec);//Get length of filename
	char* ext = strtok((char*)sec, ".");//Get part of string before dot
	//If there is no dot in the filename, length of string remains the same
	if (strlen(ext) == secret_filename_len) {
		return falha;
	}
	//Extract the extension of secret file (i.e. part of string after dot)
	ext = strtok(NULL, ".");
	strcpy((char*)encInfo->mensagem_extensao, (const char*)ext);//Store the extracted extension
	//Get and store length of secret extension
	encInfo->tamanho_msg_extensao = strlen((const char*)encInfo->mensagem_extensao);
	//Validate extension size

	free(sec);//Free the allocated block of memory
	return sucesso;//No errors found
}

Status check_capacity(EncodeInfo *encInfo)
{
	//Check if image data size is greater than magic string size
	return (encInfo->tamanho_magic_string < encInfo->tamanho_imagem) ? sucesso : falha;
}

Status do_encoding(EncodeInfo *encInfo)
{
	//Encode magic string signature
	// printf("INFO: Encoding Magic String Signature\n");
	fseek(encInfo->ptf_imagem_origem, raster_data, SEEK_SET);
	encode_magic_string((const char*)magic_string_signature, encInfo);
	
	//Encode secret file extension length
	// printf("INFO: Encoding secret file extension length\n");
	encode_int_size_expression(encInfo->tamanho_msg_extensao, encInfo);

	//Encode the dot in secret file name
	// printf("INFO: Encoding the dot in secret file name\n");
	encode_magic_string(".", encInfo);

	//Encode the secret file extension
	// printf("INFO: Encoding the secret file extension\n");
	encode_magic_string((const char*)(encInfo->mensagem_extensao), encInfo);

	//Encode the secret file size
	// printf("INFO: Encoding the secret file size\n");
	encode_int_size_expression(encInfo->tamanho_mensagem - CHAR_SIZE, encInfo);

	//Encode the secret data
	//Let's create a string to store the secret_data
	uchar_ptr secret_data = (uchar_ptr) malloc(encInfo->tamanho_mensagem * sizeof(uchar));

	rewind(encInfo->ptr_mensagem);
	fread(secret_data, encInfo->tamanho_mensagem * sizeof(uchar) - CHAR_SIZE, 1, encInfo->ptr_mensagem);
	secret_data[encInfo->tamanho_mensagem - CHAR_SIZE] = '\0';//Set last character as NUL character
	//Now let's encode secret data
	// printf("INFO: Encoding the secret data\n");
	encode_magic_string((const char*)secret_data, encInfo);

	free(secret_data);
	//Copy remaining image bytes
	// printf("INFO: Copying the left over data\n");
	copy_remaining_image_data((FILE*) encInfo->ptf_imagem_origem,(FILE*) encInfo->ptf_imagem_retorno, encInfo->tamanho_imagem - encInfo->tamanho_magic_string + CHAR_SIZE);

	return sucesso;//No error found
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
	uchar scan_char;//Read and store each byte into a character
	//Outer iteration till the size of given string
	for (uint i = 0; i < strlen(magic_string); i++)
	{
		for (int j = 7; j >= 0; j--)//8 times inner iteration, Note that iterator j should not be of the type uint.
		{//Read each byte
			fread(&scan_char, sizeof(scan_char), 1, encInfo->ptf_imagem_origem);

			scan_char &= 0xFE;//Clear the least significant bit of fetched character
			if (magic_string[i] & (01 << j))//Check every bit of magic string
			{
					scan_char |= 01;//Set the least significant bit of obtained character
			}
			else
			{
					scan_char |= 00;//Clear the least significant bit of obtained character
			}
			//Write the obtained byte onto output file
			fwrite(&scan_char, sizeof(scan_char), 1, encInfo->ptf_imagem_retorno);

		}
	}
	return sucesso;//No errors found
}

Status encode_int_size_expression(uint len, EncodeInfo *encInfo)
{       
	uchar scan_char;//Read and store each byte into a character
	for (int j = INT_SIZE * 8 - 1; j >= 0; j--)//Fetch every byte till integer size
	{
		//Read each byte
		fread(&scan_char, sizeof(scan_char), 1, encInfo->ptf_imagem_origem);

		scan_char &= 0xFE;//Clear the least significant bit of obtained character
		if (len & (1 << j))//Check every bit of obtained length
		{
				scan_char |= 01;//Set the least significant bit of obtained character
		}
		else
		{
				scan_char |= 00;//Clear the least significant bit of obtained character
		}
		//Write obtained byte onto output file
		fwrite(&scan_char, sizeof(scan_char), 1, encInfo->ptf_imagem_retorno);

	}
	return sucesso;//No errors found
}

Status copy_remaining_image_data(FILE *ptf_imagem_origem, FILE *fptr_dest_image, uint f_size)
{
	//Pointer to hold heap memory of the size of file
	uchar_ptr ch = (uchar_ptr) malloc(f_size * sizeof(uchar));
	if (ch == NULL)
	{
		// printf("ERROR: Unable to allocate dynamic memory.\n\n");
		return falha;
	}
		fread(ch, f_size, 1, ptf_imagem_origem);//Read and store all the data of file size

		fwrite(ch, f_size, 1, fptr_dest_image);//Write the obtained data onto output file

	free(ch);//Free allocated heap memory
	return sucesso;
}
