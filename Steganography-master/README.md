Para executar o código é necessário compilar, para isso utilize o comando

    gcc -Wall *.c

Esse comando irá gerar um arquivo a.out. Para executar a codificação utilize:

    ./a.out [-c] [nome da imagem] [mensagem]

Exemplo:

    ./a.out -c imagem_original.bmp mensagem.txt

Esse comando irá gerar um arquivo "imagem_codificada.bmp". Para decodificar utilize:

    ./.out [-d] [nome da imagem que deseja decodificar]

Exemplo:

    ./.out -d imagem_codificada.bmp

Esse comando irá gerar um arquivo "decodificado.txt". Ao abrir você poderá consultar a mensagem que estava escondida na imagem.