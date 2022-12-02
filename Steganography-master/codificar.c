#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codificar.h"
#include "types.h"
#include "header.h"

extern uint raster_data;
extern uchar magic_string_signature[CHAR_SIZE + CHAR_SIZE];

OperationType check_operation_type(char *argv[])
{
	if (!strcmp(*argv, "-c")) //Verifica se o comando é "-c" codificar
		return codificar;
	else if (!strcmp(*argv, "-d")) //Verifica se o comando é "-d" decodificar
		return decodificar;
	else
		return erro; // Outros comandos
}

Status copy_bmp_header(FILE *ptf_imagem_origem, FILE *ptr_arquivo_destino)
{
	// Armazena o cabeçalho da matriz (referente a imagem) na memoria
	uchar_ptr img_header = (uchar_ptr) malloc(raster_data * sizeof(uchar));
	fread(img_header, raster_data, 1, ptf_imagem_origem);
	// Grava os dados obtidos no arquivo de destino
	fwrite(img_header, raster_data, 1, ptr_arquivo_destino);
	free(img_header);
	return sucesso;
}

uint get_image_size_for_bmp(FILE *fptr_image)
{
	uint tamanho_img; // Valor a ser lido do arquivo da imagem
	// Busca o byte 34 para pegar o tamanho da imagem
	fseek(fptr_image, 34L, SEEK_SET);
	// Le a largura
	fread(&tamanho_img, sizeof(tamanho_img), 1, fptr_image);
	// Retorna o tamanho da imagem
	return tamanho_img;
}

// Retorna o tamanho da imagem incluindo o byte EOF
uint get_file_size(FILE *fptr)
{
	// Precura pelo fim do arquivo
	fseek(fptr, 0L, SEEK_END);
	// Retorna o índice
	return (uint) ftell(fptr);
}

Status open_files(EncodeInfo *encInfo)
{
	// Abre o arquivo de imagem
	encInfo->ptf_imagem_origem = fopen((const char*)encInfo->nome_imagem_origem, "rb");
	// Abre o arquivo da mensagem que será codificada
	encInfo->ptr_mensagem = fopen((const char*)encInfo->mensagem_arquivo, "rb");
	// Abre a imagem que receberá a mensagem
	encInfo->ptf_imagem_retorno = fopen((const char*)encInfo->nome_imagem_retorno, "wb");
	return sucesso;
}

Status read_and_validate_bmp_format(char *argv[])
{
	// Ponteiro para segurar o enderedo do ".bmp"
	const char* bmp_holder = strstr(*argv, ".bmp");
	if(bmp_holder)
	{
		// Se ".bmp" foi encontrado, então verifica se o arquivo realmente termina com ".bmp"
		return (!strcmp(bmp_holder, ".bmp")) ? sucesso : falha;
	}
	return falha;
}

Status read_and_validate_extn(uchar_ptr sec_file_name_holder, EncodeInfo *encInfo)
{
	// Ponteiro para segurar o tamanho do nome do arquivo
	uchar_ptr sec = (uchar_ptr) malloc(strlen((const char*)sec_file_name_holder) + 1);

	strcpy((char*)sec, (const char*)sec_file_name_holder); // Armazena o nome do arquivo
	uint secret_filename_len = strlen((const char*)sec); // Pega o tamanho do nome do arquivo
	char* ext = strtok((char*)sec, "."); // Pega a parte da string antes do ponto (.)
	if (strlen(ext) == secret_filename_len) {
		return falha;
	}
	// Extrai a extensão do arquivo de mensagem
	ext = strtok(NULL, ".");
	strcpy((char*)encInfo->mensagem_extensao, (const char*)ext); // Armazena a extensão
	// Pega e armazena o tamanho da extensão
	encInfo->tamanho_msg_extensao = strlen((const char*)encInfo->mensagem_extensao);
	// Valida o tamanho da extensão

	free(sec);
	return sucesso;
}

Status check_capacity(EncodeInfo *encInfo)
{
	// Verifica se o tamanho da imagem é maior que o da magic string
	return (encInfo->tamanho_magic_string < encInfo->tamanho_imagem) ? sucesso : falha;
}

Status do_encoding(EncodeInfo *encInfo)
{
	// Codifica a assinatura da magic string
	fseek(encInfo->ptf_imagem_origem, raster_data, SEEK_SET);
	encode_magic_string((const char*)magic_string_signature, encInfo);
	
	// Codifica o tamanho da extensão do arquivo da mensagem
	encode_int_size_expression(encInfo->tamanho_msg_extensao, encInfo);

	// Codifica o ponto (.) no nome do arquivo da mensagem
	encode_magic_string(".", encInfo);

	// Codifica a extensão do arquivo da mensagem
	encode_magic_string((const char*)(encInfo->mensagem_extensao), encInfo);

	// Codifica o tamanho da mensagem
	encode_int_size_expression(encInfo->tamanho_mensagem - CHAR_SIZE, encInfo);

	// Codifica a mensagem
	uchar_ptr secret_data = (uchar_ptr) malloc(encInfo->tamanho_mensagem * sizeof(uchar));

	rewind(encInfo->ptr_mensagem);
	fread(secret_data, encInfo->tamanho_mensagem * sizeof(uchar) - CHAR_SIZE, 1, encInfo->ptr_mensagem);
	secret_data[encInfo->tamanho_mensagem - CHAR_SIZE] = '\0'; // Seta o ultimo caracter como NULL

	// Codificando a mensagem 
	encode_magic_string((const char*)secret_data, encInfo);

	free(secret_data);

	// Copia os bytes restantes da imagem
	copy_remaining_image_data((FILE*) encInfo->ptf_imagem_origem,(FILE*) encInfo->ptf_imagem_retorno, encInfo->tamanho_imagem - encInfo->tamanho_magic_string + CHAR_SIZE);

	return sucesso;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
	uchar scan_char; // Le e armazena cada byte em um caracter

	for (uint i = 0; i < strlen(magic_string); i++)
	{
		for (int j = 7; j >= 0; j--)
		{
			// Le cada byte
			fread(&scan_char, sizeof(scan_char), 1, encInfo->ptf_imagem_origem);

			scan_char &= 0xFE; // Limpa o byte menos significante
			if (magic_string[i] & (01 << j)) // Verifica cada byte da magic string
			{
					scan_char |= 01; // Define o bit menos significante do caracter obtido
			}
			else
			{
					scan_char |= 00; // Limpa o byte menos significante do caracter obtido
			}
			// Escreve o byte obtido no arquivo de saída (arquivo codificado)
			fwrite(&scan_char, sizeof(scan_char), 1, encInfo->ptf_imagem_retorno);

		}
	}
	return sucesso;
}

Status encode_int_size_expression(uint len, EncodeInfo *encInfo)
{      
	uchar scan_char; 
	// Le e armazena cada byte em um caracter
	for (int j = INT_SIZE * 8 - 1; j >= 0; j--)
	{
		// Le cada byte
		fread(&scan_char, sizeof(scan_char), 1, encInfo->ptf_imagem_origem);

		scan_char &= 0xFE; // Limpa o byte menos significante do caracter obtido
		if (len & (1 << j)) // Verifica cada bit do comprimento obtido
		{
				scan_char |= 01; // Define o bit menos significante do caracter obtido
		}
		else
		{
				scan_char |= 00; // Limpa o byte menos significante do caracter obtido
		}
		// Escreve o byte obtido no arquivo de saída (arquivo codificado)
		fwrite(&scan_char, sizeof(scan_char), 1, encInfo->ptf_imagem_retorno);

	}
	return sucesso;
}

Status copy_remaining_image_data(FILE *ptf_imagem_origem, FILE *ptr_arquivo_destino, uint f_size)
{
	// Ponteiro para manter a memória do tamanho do arquivo
	uchar_ptr ch = (uchar_ptr) malloc(f_size * sizeof(uchar));
	if (ch == NULL)
	{
		return falha;
	}
		fread(ch, f_size, 1, ptf_imagem_origem); // Le e guarda todos os dados

		fwrite(ch, f_size, 1, ptr_arquivo_destino); // Escreve os dados no arquivo de saída

	free(ch);
	return sucesso;
}
