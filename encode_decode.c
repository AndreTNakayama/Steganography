#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codificar.h"
#include "types.h"
#include <libgen.h>

//Armazena o indice do raster data, do arquivo, comprimento da mensagem
uint raster_data, secret_filename_len, default_ext_name = 0;
uchar magic_string_signature[CHAR_SIZE + CHAR_SIZE];
uchar temp_decode_name[FILENAME_SIZE];

int main(int argc, char* argv[]) {
	EncodeInfo encInfo;
	if (argc < 3) {
		exit(sucesso);
	}
	if (check_operation_type(argv + 1) == codificar){
		if (argc < 4 || argc > 7) {
			exit(sucesso);
		}
		// Leia e valide o nome do arquivo
		// Extraia apenas o nome do arquivo do caminho fornecido
		argv[2] = basename(argv[2]);
		if (read_and_validate_bmp_format(argv + 2) == falha) {
			exit(sucesso);
		}
		strcpy((char*)encInfo.nome_imagem_origem, argv[2]);
		// Valide o terceiro argumento como nome de arquivo codificado

		// Extraia apenas o nome do arquivo fornecido
		argv[3] = basename(argv[3]);
		read_and_validate_extn((uchar_ptr)argv[3], &encInfo);

		strcpy((char*)encInfo.mensagem_arquivo, argv[3]);
		switch (argc) {
			case 4:
				// Usando o nome de arquivo de saída padrão, já que nenhum argumento é fornecido
				strcpy((char*)encInfo.nome_imagem_retorno, "imagem_codificada.bmp");
				break;
			case 5:
				// Le e valida o nome de arquivo de saída fornecido
				argv[4] = basename(argv[4]);
				if (read_and_validate_bmp_format(argv + 4) == falha) {
					exit(sucesso);
				}
				strcpy((char*)encInfo.nome_imagem_retorno, argv[4]);
				break;
			default:
				//Le e valida o nome de arquivo de saída fornecido
				argv[4] = basename(argv[4]);
				if (read_and_validate_bmp_format(argv + 4) == falha) {
					exit(sucesso);
				}
				strcpy((char*)encInfo.nome_imagem_retorno, argv[4]);
		}
		// Tenta abrir arquivos
		open_files(&encInfo);

		// Deslocamento de dados: procurar o 10º índice do arquivo bmp
		fseek(encInfo.ptf_imagem_origem, 10L, SEEK_SET);
		fread(&raster_data, sizeof(raster_data), 1, encInfo.ptf_imagem_origem);

		rewind(encInfo.ptf_imagem_origem);
		// Copiar cabeçalho da imagem
		copy_bmp_header((FILE*) encInfo.ptf_imagem_origem,(FILE*) encInfo.ptf_imagem_retorno);

		// O tamanho dos dados da imagem deve ser maior que o tamanho da magic string
		encInfo.tamanho_imagem = get_image_size_for_bmp(encInfo.ptf_imagem_origem); 

		encInfo.tamanho_mensagem = get_file_size(encInfo.ptr_mensagem);

		//Magic string = MSS + Tamanho da extensão da mensagem + extensão + tamanho da mensagem + mensagem 
		encInfo.tamanho_magic_string = (CHAR_SIZE + INT_SIZE + CHAR_SIZE + encInfo.tamanho_msg_extensao + INT_SIZE + encInfo.tamanho_mensagem - CHAR_SIZE) * 8;
		strcpy((char*)magic_string_signature, MAGIC_STRING);
		
		// Verifique a capacidade de codificação
		check_capacity(&encInfo);
		do_encoding(&encInfo);

		fclose(encInfo.ptf_imagem_origem); // Fechar arquivo de imagem original
		fclose(encInfo.ptr_mensagem); // Fecha a mensagem
	}
	else if (check_operation_type(argv + 1) == decodificar){ // Decodificar
		if (argc > 6) {
			exit(sucesso);
		}
		// Lê e valida o nome do arquivo de saída fornecido
		argv[2] = basename(argv[2]);
		read_and_validate_bmp_format(argv + 2);

		strcpy((char*)encInfo.nome_imagem_retorno, argv[2]);
		switch (argc) {
			// argc = 3, decodificação da extensão do arquivo
			case 3: default_ext_name = 1;
				break;
			case 4: // Usuário define o nome do arquivo decodificado
				argv[3] = basename(argv[3]);
				strcpy((char*)temp_decode_name, argv[3]);
				break;
			default:
				argv[3] = basename(argv[3]);
				strcpy((char*)temp_decode_name, argv[3]);
		}
		// Abrir imagem decodificada
		encInfo.ptf_imagem_retorno = fopen((const char*)encInfo.nome_imagem_retorno, "rb");
		// Coletar deslocamento de dados

		fseek(encInfo.ptf_imagem_retorno, 10L, SEEK_SET);
		fread(&raster_data, sizeof(raster_data), 1, encInfo.ptf_imagem_retorno);
		fseek(encInfo.ptf_imagem_retorno, raster_data, SEEK_SET);

		// O índice do arquivo decodificado agora está apontando para o final dos dados
		// Decodificar assinatura da magic string
		do_decoding(&encInfo);

		// Fechar arquivo de saída decodificado
		fclose(encInfo.fptr_decoded_file);
	} else {
		// Erro
		exit(sucesso);
	}
	// Feche o arquivo de saída
	fclose(encInfo.ptf_imagem_retorno);
	return 0;
}
