<!-- Este projeto pode ser encontrado no GitHub atráves deste link: https://github.com/hmarcuzzo/project_B_SO -->
# project_B_SO
########### SHELL.C ##########


COMO COMPILAR
=============

- Executar o MakeFile, utilizando o comando 'make'

COMO EXECUTAR
=============

- Utilizar o comando './shell.o'

BIBLIOTECAS UTILIZADAS
======================

- <stdio.h> 
- <string.h> 
- <stdlib.h> 
- <string.h>
- <math.h>

- <unistd.h> * Compatibilidade com UNIX (POSIX)

- <sys/types.h> * Utilizadada para o tipo de dado 'pid_t'

- <sys/wait.h> * Utilizada para a função 'wait()'

- <readline/readline.h> 
    * Utilizada para a função 'readline()'
    * Caso necessário, instale a biblioteca GNU Readline com o comando: 'sudo apt-get install lib32readline7 lib32readline-dev'

EXEMPLOS DE USO
===============

Funções normais do shell:
=========================

- Execução em backgroud: "sleep 2&" , "ls&"

- Exibir o histórico: "history"

- Executar determinado comando da lista de histórico: "!4"

- Executar úlimo comando: "!!"

No disco virtual:
=================

Obs: Para entrar no disco virtual é necessário utilizar ```cd disco.dsc``` e para sair, deve-se digitar ```exit```

- cd \<dir> (vai para um diretório)

- mkdir \<dir> (criar diretórios)

- rm <dir | arquivo> (remove arquivos ou diretórios)

- cp \<arquivo> .real (copiar arquivo do real para o virtual, podendo ser só o arquivo ou o caminho com o nome do arquivo no final ex.: ```cp caminho/arquivo.txt .real```)

- cp \<arquivo> \<arquivo> .virtual (copiar arquivo do virtual para o real, sendo os parâmetros: nome no arquivo no virtual, e o caminho com o nome do arquivo no final ex.: ```cp arquivo.txt caminho/arquivo2.txt .virtual```)

- mv <dir | arquivo> (move arquivo ou pasta de diretório)

- ls \<dir> (lista o conteúdo do diretorório)

- pwd (mostra o caminho do diretório atual)

- format dsc (formatar o disco)

- exit (sair do disco virtual)