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

- <unistd.h> * Compatibilidade com UNIX (POSIX)

- <sys/types.h> * Utilizadada para o tipo de dado 'pid_t'

- <sys/wait.h> * Utilizada para a função 'wait()'

- <readline/readline.h> 
    * Utilizada para a função 'readline()'
    * Caso necessário, instale a biblioteca GNU Readline com o comando: 'sudo apt-get install lib32readline7 lib32readline-dev'

EXEMPLOS DE USO
===============

- Execução em backgroud: "sleep 2&" , "ls&"

- Exibir o histórico: "history"

- Executar determinado comando da lista de histórico: "!4"

- Executar úlimo comando: "!!"
