#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "header.h"

//Extern required variables from main file
extern uint default_ext_name;
extern uchar temp_decode_name[FILENAME_SIZE]; 

Status do_decoding(EncodeInfo* encInfo)
{
	uchar_ptr receive_str = NULL;//Pointer to decoded string	
	// printf("INFO: Decoding Magic String Signature\n");
	//If first decoded byte is * then no pass code. If 1st byte is # then pass code expected
	receive_str = decode_magic_string(CHAR_SIZE, encInfo);

	uint dec_extn_size = decode_int_size_expression(encInfo);

	receive_str = decode_magic_string(CHAR_SIZE, encInfo);

	//Decode File extension
	receive_str = decode_magic_string(dec_extn_size, encInfo);
	if (receive_str)
	{
		if (default_ext_name)
		{
			strcpy((char*) encInfo->decoded_fname, (const char*) "decodificado.");
			strcat((char*)encInfo->decoded_fname, (const char*)receive_str);
		}
		else
		{

			strcpy((char*)encInfo->decoded_fname, (const char*)temp_decode_name);

		}
	}
	else
	{
		return falha;
	}
	//Decode secret data size
	encInfo->tamanho_mensagem = decode_int_size_expression(encInfo);

	//Let's open the decode file for writing the secret data
	encInfo->fptr_decoded_file = fopen((const char*)encInfo->decoded_fname, "wb");

	//Decode file data
	decode_file_data(encInfo->tamanho_mensagem, encInfo);

	free(receive_str);
	return sucesso;//No error found
}

uchar_ptr decode_magic_string(uint size, EncodeInfo* encInfo)
{
	//Pointer to hold the heap memory of given size
	uchar_ptr decoded_str = (uchar_ptr) malloc(size* sizeof(uchar));

	uchar scan_char;//Read and store each character
	uint j;//Outer iterator
	for (j = 0; j < size; j++)//Iterate till given string size
	{
		uchar ch = 0;//To store every obtained byte
		for (uint i = 0; i < 8; i++)//8 times inner iteration
		{
			//Read and store each byte
			fread(&scan_char, sizeof(scan_char), 1, encInfo->ptf_imagem_retorno);
			scan_char &= 01;//Obtain the least significant bit
			ch <<= 1;//Left shift by 1 bit to store obtained least significant bit
			ch |= scan_char;//Store the obtained least significant bit
		}
		decoded_str[j] = ch;//Store obtained character into heap
	}
	decoded_str[j] = '\0';//Append NUL character in the end.
	return decoded_str;//Return obtained string
	//Free the heap memory at caller side after executing this function
}

uint decode_int_size_expression(EncodeInfo* encInfo)
{
	uint decoded_int = 0;//To store decoded integer value
	for (uint j = 0; j < INT_SIZE; j++)//Integer size outer iterations
	{
		uchar scan_char = 0;//Read and store each character
		for (uint i = 0; i < 8; i++)//8 times inner iteration
		{
			//Read and store each byte
			fread(&scan_char, sizeof(scan_char), 1, encInfo->ptf_imagem_retorno);

			scan_char &= 01;//Obtain the least significant bit
			decoded_int <<= 1;//Left shift by 1 bit to store obtained least significant bit
			decoded_int |= (uint) scan_char;//Store the obtained least significant bit
		}
	}
	return decoded_int;//Return obtained integer
}

Status decode_file_data(uint f_size, EncodeInfo* encInfo)
{
	//Pointer to hold the heap memory of file size
	uchar_ptr file_data = (uchar_ptr) malloc(f_size * sizeof(uchar));

	uchar scan_char;//Read and store each character
	for (uint j = 0; j < f_size; j++)//File size outer iterations
	{
		uchar ch = 0;//To store every obtained byte
		for (uint i = 0; i < 8; i++)//8 times inner iteration
		{
			//Read and store each byte from image file
			fread(&scan_char, sizeof(scan_char), 1, encInfo->ptf_imagem_retorno);

			scan_char &= 01;//Obtain the least significant bit
			ch <<= 1;//Left shift by 1 bit to store obtained least significant bit
			ch |= scan_char;//Store the obtained least significant bit
		}
		file_data[j] = ch;//Store obtained character into heap
	}
	//Write obtained heap data onto decoded file
	fwrite(file_data, f_size, 1, encInfo->fptr_decoded_file);

	free(file_data);//Free allocated heap memory
	return sucesso;
}
