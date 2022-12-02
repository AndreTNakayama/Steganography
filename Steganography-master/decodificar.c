#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codificar.h"
#include "types.h"
#include "header.h"

extern uint default_ext_name;
extern uchar temp_decode_name[FILENAME_SIZE]; 

Status do_decoding(EncodeInfo* encInfo)
{
	uchar_ptr receive_str = NULL; // Ponteiro para decodificar o texto
	receive_str = decode_magic_string(CHAR_SIZE, encInfo);
	uint dec_extn_size = decode_int_size_expression(encInfo);
	receive_str = decode_magic_string(CHAR_SIZE, encInfo);

	// Decodificar extensão do arquivo
	receive_str = decode_magic_string(dec_extn_size, encInfo);
	if (receive_str)
	{
		if (default_ext_name)
		{
			strcpy((char*) encInfo->nome_arquivo_decodificar, (const char*) "decodificado.");
			strcat((char*)encInfo->nome_arquivo_decodificar, (const char*)receive_str);
		}
		else
		{

			strcpy((char*)encInfo->nome_arquivo_decodificar, (const char*)temp_decode_name);
		}
	}
	else
	{
		return falha;
	}
	// Decodificar o tamanho da mensagem
	encInfo->tamanho_mensagem = decode_int_size_expression(encInfo);

	// Vamos abrir o arquivo de decodificação para gravar os dados
	encInfo->ptr_decodificar_arquivo = fopen((const char*)encInfo->nome_arquivo_decodificar, "wb");

	// Decodificar dados do arquivo
	decode_file_data(encInfo->tamanho_mensagem, encInfo);

	free(receive_str);
	return sucesso;
}

uchar_ptr decode_magic_string(uint size, EncodeInfo* encInfo)
{
	// Ponteiro para manter a memória de determinado tamanho
	uchar_ptr decoded_str = (uchar_ptr) malloc(size* sizeof(uchar));

	uchar scan_char; // Ler e armazenar cada caractere
	uint j;
	for (j = 0; j < size; j++)
	{
		uchar ch = 0;// Armazenar cada byte obtido
		for (uint i = 0; i < 8; i++)
		{
			// Ler e armazenar cada byte
			fread(&scan_char, sizeof(scan_char), 1, encInfo->ptf_imagem_retorno);
			scan_char &= 01; // Obter o bit menos significativo
			ch <<= 1; // Deslocar à esquerda em 1 bit para armazenar o bit menos significativo obtido
			ch |= scan_char; // Armazena o bit menos significativo obtido
		}
		decoded_str[j] = ch; // Armazene o caractere obtido
	}
	decoded_str[j] = '\0';// Adicione o caractere NUL no final.
	return decoded_str; // Retorna a string obtida
}

uint decode_int_size_expression(EncodeInfo* encInfo)
{
	uint decoded_int = 0; // Para armazenar o valor inteiro decodificado
	for (uint j = 0; j < INT_SIZE; j++)
	{
		uchar scan_char = 0; // Ler e armazenar cada caractere
		for (uint i = 0; i < 8; i++)
		{
			// Armazenar cada byte obtido
			fread(&scan_char, sizeof(scan_char), 1, encInfo->ptf_imagem_retorno);

			scan_char &= 01; // Obter o bit menos significativo
			decoded_int <<= 1; // Deslocar à esquerda em 1 bit para armazenar o bit menos significativo obtido
			decoded_int |= (uint) scan_char; // Armazena o bit menos significativo obtido
		}
	}
	return decoded_int; // Retorna o inteiro obtido
}

Status decode_file_data(uint f_size, EncodeInfo* encInfo)
{
	// Ponteiro para manter a memória de determinado tamanho
	uchar_ptr file_data = (uchar_ptr) malloc(f_size * sizeof(uchar));

	uchar scan_char; // Ler e armazenar cada caractere
	for (uint j = 0; j < f_size; j++)
	{
		uchar ch = 0; // Armazenar cada byte obtido
		for (uint i = 0; i < 8; i++)
		{
			// Ler e armazenar cada byte
			fread(&scan_char, sizeof(scan_char), 1, encInfo->ptf_imagem_retorno);

			scan_char &= 01; // Obter o bit menos significativo
			ch <<= 1; // Deslocar à esquerda em 1 bit para armazenar o bit menos significativo obtido
			ch |= scan_char; //Armazenar  o bit menos significativo
		}
		file_data[j] = ch; // Armazena o caracter obtido
	}
	// Grave os dados obtidos no arquivo decodificado
	fwrite(file_data, f_size, 1, encInfo->ptr_decodificar_arquivo);

	free(file_data);
	return sucesso;
}