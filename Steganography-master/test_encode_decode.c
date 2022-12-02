#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include <libgen.h>

/* Global variables. These will be externed into other files */
//Store raster data index, file index, length of seret filename
//Flags indicating need of passcode and need of default file extension
uint raster_data, secret_filename_len, default_ext_name = 0;
//string for storing magic string signature: * or # with '\0' character
uchar magic_string_signature[CHAR_SIZE + CHAR_SIZE];
//string storing temporary user provided decode filename
uchar temp_decode_name[FILENAME_SIZE];

int main(int argc, char* argv[]) {
	EncodeInfo encInfo;//Structure variable
	if (argc < 3) {
		exit(sucesso);
	}
	if (check_operation_type(argv + 1) == codificar){
		if (argc < 4 || argc > 7) {
			exit(sucesso);
		}
		//Read and validate src_image filename
		//Extract only filename from given path if any
		argv[2] = basename(argv[2]);
		if (read_and_validate_bmp_format(argv + 2) == falha) {
			exit(sucesso);
		}
		strcpy((char*)encInfo.nome_imagem_origem, argv[2]);
		//Validate and assign 3rd command line argument as secret filename

		//Extract only filename from given path if any
		argv[3] = basename(argv[3]);
		read_and_validate_extn((uchar_ptr)argv[3], &encInfo);

		strcpy((char*)encInfo.mensagem_arquivo, argv[3]);
		switch (argc) {
			case 4:
				//Giving default output filename since no 4th command line argument is given
				strcpy((char*)encInfo.nome_imagem_retorno, "imagem_codificada.bmp");
				// printf("INFO: No output filename given. Creating default output image file %s in current project directory\n", encInfo.nome_imagem_retorno);
				break;
			case 5:
				//Read and validate given output filename
				//Extract only filename from given path if any
				argv[4] = basename(argv[4]);
				if (read_and_validate_bmp_format(argv + 4) == falha) {
					exit(sucesso);
				}
				strcpy((char*)encInfo.nome_imagem_retorno, argv[4]);
				break;
			default://case 7
				//Read and validate given output filename
				//Extract only filename from given path if any
				argv[4] = basename(argv[4]);
				if (read_and_validate_bmp_format(argv + 4) == falha) {
					exit(sucesso);
				}
				strcpy((char*)encInfo.nome_imagem_retorno, argv[4]);
		}
		// Test open_files
		open_files(&encInfo);

		//Collect raster data offset: seek to 10th index of bmp file
		fseek(encInfo.ptf_imagem_origem, 10L, SEEK_SET);
		fread(&raster_data, sizeof(raster_data), 1, encInfo.ptf_imagem_origem);

		rewind(encInfo.ptf_imagem_origem);
		//Copy image header
		copy_bmp_header((FILE*) encInfo.ptf_imagem_origem,(FILE*) encInfo.ptf_imagem_retorno);

		//Image data size should be larger than Magic String Size
		encInfo.tamanho_imagem = get_image_size_for_bmp(encInfo.ptf_imagem_origem); 

		encInfo.tamanho_mensagem = get_file_size(encInfo.ptr_mensagem);

		//Magic string = MSS + secret file extn size + dot + file extn + secret data size + secret data 
		encInfo.tamanho_magic_string = (CHAR_SIZE + INT_SIZE + CHAR_SIZE + encInfo.tamanho_msg_extensao + INT_SIZE + encInfo.tamanho_mensagem - CHAR_SIZE) * 8;
		strcpy((char*)magic_string_signature, MAGIC_STRING);
		
		//Check encoding capacity
		check_capacity(&encInfo);
		do_encoding(&encInfo);

		fclose(encInfo.ptf_imagem_origem);//close source image file
		fclose(encInfo.ptr_mensagem);//Close secret file
	}
	else if (check_operation_type(argv + 1) == decodificar){//Decoding
		if (argc > 6) {
			exit(sucesso);
		}
		//Read and validate given output filename
		// printf("INFO: Verifying image filename..\n");
		//Extract only filename from given path if any
		argv[2] = basename(argv[2]);
		read_and_validate_bmp_format(argv + 2);

		strcpy((char*)encInfo.nome_imagem_retorno, argv[2]);
		switch (argc) {
			//For argc = 3, wait for file extension decoding
			case 3: default_ext_name = 1;
				break;
			case 4: //User defined decoded filename
				//Extract only filename from given path if any
				argv[3] = basename(argv[3]);
				strcpy((char*)temp_decode_name, argv[3]);
				break;
			default: // argc = 6, User defined decoded filename, -p and pass code
				//Extract only filename from given path if any
				argv[3] = basename(argv[3]);
				strcpy((char*)temp_decode_name, argv[3]);
		}
		//Open stegged image file
		encInfo.ptf_imagem_retorno = fopen((const char*)encInfo.nome_imagem_retorno, "rb");
		//Collect raster data offset

		fseek(encInfo.ptf_imagem_retorno, 10L, SEEK_SET);
		fread(&raster_data, sizeof(raster_data), 1, encInfo.ptf_imagem_retorno);
		fseek(encInfo.ptf_imagem_retorno, raster_data, SEEK_SET);

		//Stego file index is now pointing at the end of raster data
		//Decode Magic String Signature
		do_decoding(&encInfo);

		//close decoded output file
		fclose(encInfo.fptr_decoded_file);
	} else {//e_unsupported - Neither encoding nor decoding option
		exit(sucesso);
	}
	//Close the output file
	fclose(encInfo.ptf_imagem_retorno);//common file both in encoding & decoding part
	return 0;
}
