/*********************************************************************
* NOME DO ARQUIVO :        shell.c                                   *
*                                                                    *
* DESCRIÇÂO :                                                        *
*                                                                    *
*       Este programa consiste em uma interface shell capaz de       *
*       executar processos por linha de comando. Também possui       *
*       as funcionalidades de histórico e execução em background.    *
*                                                                    *
* AUTOR :    Matheus Henrique Batistela                              *
*                                                                    *
* DATA DE CRIAÇÃO :    11/09/2019                                    *
*                                                                    *
* MODIFICAÇÕES :       15/12/2019                                    *
*                                                                    *
**********************************************************************/

#include<stdio.h> 
#include<string.h>
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include<readline/readline.h>
#include<math.h>
#include"sist.h"

#define MAXLINE 80 // Define o tamanho máximo da linha de comando
#define MAXARGS 10 // Define o número máximo de argumentos lidos por linha de comando
#define MAXHISTORY 10 // Define o máximo de comandos armazenados no histórico

typedef enum 
{
    EXIT=1,CD,HELP,HISTORY,LASTCMD,NCMD
}builtInCmds;

typedef enum
{
    EXITV=1, CDV, MKDIRV, RMV, CPV, MVV, LSV, PWDV, FORMATV
}virtualCmds;

int virtual = 0;

char history[MAXHISTORY][MAXLINE]; // Vetor de strings que armazena uma linha de comando em cada índice
int history_counter; // Contador do número de comandos salvos no histórico

int parseArg(char* cmdline, char** parsed);
void addHistory(char *buff_cmd);
int isBuiltIn(char **parsed_cmd);
void execBuiltIn(int cmd,char **parsed_cmd);
int isBackground(char* last_parse);
void execCmd(char** parsed, int background);
void execLine(char *cmdline);

// Separa os argumentos da string "cmdline" quebrando-a onde houver um espaço e atribuindo os valores ao ponteiro "parsed"
// sempre terminado em NULL 
int parseArg(char* cmdline, char** parsed) 
{
    if (cmdline[-1] == '\n' && cmdline[-1] == '\0')
        return 0; 
    
    char *cmdline_copy = (char*)calloc(MAXLINE,sizeof(char));
    strcpy(cmdline_copy,cmdline); 
    for (int i = 0; i < MAXLINE; i++) { 
        parsed[i] = strsep(&cmdline_copy, " ");
        if (parsed[i] == NULL) 
            return i; 
    }
}

// Adiciona no vetor de histórico os comandos juntos com seus argumentos digitados na linha de comando. Esta função não
// inclui os comandos para acessar o histórico
void addHistory(char *buff_cmd)
{
    if((strcmp(buff_cmd,"history") == 0 || buff_cmd[0] == '!')) // Verifica se o comando passado é do próprio histórico,
        return; // caso for, retorna e não adiciona
    else if(history_counter < MAXHISTORY) // Caso o histórico não tenha atingido seu valor máximo
    {
        strcpy(history[history_counter],buff_cmd); // Copia o camndo para o histórico
        history_counter++; // Incrementa 1 no contador de comandos
    }
    else if(history_counter == 10) // Caso o histórico tenha atingido seu limite
    {
        for (int i = 0 ; i < MAXHISTORY -1; i++)
            strcpy(history[i],history[i+1]); // Faz um "shift" para a esquerda no vetor de comandos
        
        strcpy(history[history_counter-1],buff_cmd); // Copia a novo comando para a ultima posição
    }
}

// Executa num processo filho o comando passado por parâmetro junto com seus argumentos 
void execCmd(char** parsed,int background) 
{ 
    pid_t pid = fork(); // Cria o processo filho

    if(pid == -1) 
    {
        printf("Processo filho não executado!\n");
        return;
    } 
    else if(pid == 0)
    {
        int ex = execvp(parsed[0], parsed); // Executa o comando no processo filho
        
        if(ex < 0)
            printf("Comando não encontrado\n");
    }
    else if (background) // Caso for background, não espera pelo filho
    {
        printf("Uhhhggr zombie process\n");
        return;
    }
    else
    {
        waitpid(pid,NULL,0); // Espera a execução do filho
        return;
    }
       
}

// Verifica se o último ou único argumento passado possui uma flag de exec em backgrouond
int isBackground(char* last_parse)
{   
    int index = strlen(last_parse) -1; // Guarda a última posição do parse do argumento
    if (last_parse[index] == '&') // Verifica se a última posição contém um '&'
    {
        memmove(&last_parse[index],&last_parse[index+1],strlen(last_parse) - index); // Remove a flag para uma exec coreta
        return 1;
    }
    return 0;
}

// Esta função processa a linha e a analisa, decidindo o tipo de execução
void execLine(char *cmdline)
{
    char *parsed_cmd[MAXARGS]; // Armazena , separadamente, o comando e seus argumentos
    int n_parse;
    if(n_parse = parseArg(cmdline,parsed_cmd)) // Verifica se o parse foi efetuado com sucesso, caso sim, guarda o resultado no segundo parâmetro
    {
        
        int builtin_cmd = isBuiltIn(parsed_cmd);
        int background = isBackground(parsed_cmd[n_parse-1]);

        if(builtin_cmd) // Verifica se o comando é interno
        {
            execBuiltIn(builtin_cmd, parsed_cmd); // Executa o comando interno
        }
        else
        {
            execCmd(parsed_cmd,background); // Executa o comando externo
        }
    }
}

// Executa o comando interno passado por parâmetro
void execBuiltIn(int cmd,char **parsed_cmd)
{
    int index;
    if(virtual == 1){
        switch (cmd)
        {
            case EXITV:
                virtual = 0;                                                // sai das funções do disco virtual
                break;
            case CDV:
                cd(parsed_cmd[1]);                                          // entra em uma pasta no disco virtual
                break;
            case MKDIRV:
                mkdir(parsed_cmd[1],parsed_cmd[2]);                         // cria uma pasta no disco virtual
                break;
            case RMV:
                removeItem(parsed_cmd[1], actualDir[adpCount - 1]);         // apaga um item um item do disco virtual
                break;
            case CPV:
                if(strcmp(parsed_cmd[2],".real")==0){                       // copia um arquivo do disco real para o virtual
                    writeInDisk(parsed_cmd[1], actualDir[adpCount - 1]);
                }else
                if(strcmp(parsed_cmd[3],".virtual")==0){                    // copia um arquivo do disco virtual para o real
                    readFromDisk(parsed_cmd[1], parsed_cmd[2]);
                }
                break;
            case MVV:
                move(parsed_cmd[2], parsed_cmd[1]);                         // move um arquivo dentro do disco virtual
                break;
            case LSV:
                if(parsed_cmd[1] == NULL){
                    printDir(FAT[actualDir[adpCount - 1]]);                 // mostra todos os arquivos do diretório atual no disco virtual
                }
                break;
            case PWDV:
                pwd();                                                      // mostra o caminho do diretório atual
                break;
            case FORMATV:
                if(strcmp(parsed_cmd[2],"dsc")==0){
                    startFileSystem();                                          // apaga todo o conteúdo do disco virtual
                }
                break;
            default:
                printf("Command not found.");
        }
    }else{
        switch (cmd)
        {
            case EXIT:
                exit(0);
                break;
                
            case CD:
                if (parsed_cmd[1][strlen(parsed_cmd[1]) - 1] == 'c' && parsed_cmd[1][strlen(parsed_cmd[1]) - 2] == 's' && parsed_cmd[1][strlen(parsed_cmd[1]) - 3] == 'd'){
                    virtual = 1;
                    startFileSystem();
                    break;
                }
                if (chdir(parsed_cmd[1]) == -1) // Utiliza-se da função chdir() que acessa o direório passado por argumento
                    printf(" %s : Diretório inexistente\n", parsed_cmd[1]);
                    break;
                
            case HELP: 
                printf("\n1. Digite o comando desejado e tecle enter para executá-lo\n\n");
                printf("2. Use o comando 'history' para ver o histórico de comandos\n\n");
                printf("3. Use o comando '!!' para executar o último comando\n\n");
                printf("4. Use o comando '!n' para executar o último n comando desejado\n\n");
                printf("5. Use '&' no final do comando para executá-lo em background\n\n");
                break;

            case HISTORY:
                printf("\n");
                int cmd_number = history_counter +1;
                for(int i=0; i < history_counter ; i++ ){
                    cmd_number--;
                    printf("%d %s\n",cmd_number,history[i]);
                }
                printf("\n");
                break;

            case LASTCMD:
                execLine(history[history_counter-1]); // Executa a última linha armazenada no histórico
                break;

            case NCMD:
                index = (parsed_cmd[0])[1] - '0'; // Converte e armazena a segunda posição do arg, a qual, contém o número do proc a ser executado
                index = history_counter - index; // Subtrai do contador do histórico para descobrir o índice correto
                execLine(history[index]); // Executa a linha correspondente à posição escollhida
                break;
    
            default:
                break;
        }
    }
}

// Verifica se o comando digitado permanece a grupo de comandos internos
int isBuiltIn(char **parsed_cmd)
{
    int n_builtin = 6; // Número de comandos internos
    char *builtin_commands[n_builtin]; // Vetor que contém os comandos internos
    int command;
    
    builtin_commands[0] = "exit";
    builtin_commands[1] = "cd";
    builtin_commands[2] = "help";
    builtin_commands[3] = "history";
    builtin_commands[4] = "!!";
    builtin_commands[5] = "!";
    
    int n_virtual = 9;
    char *virtual_commands[n_virtual];
    virtual_commands[0] = "exit";
    virtual_commands[1] = "cd";
    virtual_commands[2] = "mkdir";
    virtual_commands[3] = "rm";
    virtual_commands[4] = "cp";
    virtual_commands[5] = "mv";
    virtual_commands[6] = "ls";
    virtual_commands[7] = "pwd";
    virtual_commands[8] = "format";

    if(virtual == 1){
        for(int i=0; i < n_virtual - 1; i++){
            if(strcmp(parsed_cmd[0],virtual_commands[i]) == 0) // Compara o comando passado com todos os internos
                return i+1; // Caso for igual, retorna o número corresponde àquele comando
        }
    }else{
        for(int i=0; i < n_builtin -1; i++)
        {
            if(strcmp(parsed_cmd[0],builtin_commands[i]) == 0) // Compara o comando passado com todos os internos
                return i+1; // Caso for igual, retorna o número corresponde à aquele comando
        }
    }

    // Compara o primeiro caractere do comando com '!' para verificar se é uma execução do histórico
    if((parsed_cmd[0])[0] == *(builtin_commands[5]))
        return NCMD;
    else
        return 0;
}


// Função principal
int main(int argc, char const *argv[])
{
    char cmdline[MAXLINE]; // Armazena a linha de comando (Input do usuário)
    
    while (1)
    {
        if(virtual == 1){
            printf("Virtual disk >> ");
            
            char *buffer;
            buffer = readline(NULL); // Capta o input do usuário
 
            if(strlen(buffer) != 0)
            {
                strcpy(cmdline,buffer); // Armazena o conteúdo do buffer
                execLine(cmdline); // Executa o comando
                addHistory(cmdline); // Adiciona o comando no histórico
            }    
        }else{
            char host[1024] = "";
            char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            gethostname(host, sizeof(host)); // Capta o nome do host e armazena na variável "host"
            printf("%s@%s:%s$ ", getenv("LOGNAME"), host, cwd); // Printa o hostname, juntamente com o nome da máquina

            char *buffer;
            buffer = readline(NULL); // Capta o input do usuário
 
            if(strlen(buffer) != 0)
            {
                strcpy(cmdline,buffer); // Armazena o conteúdo do buffer
                execLine(cmdline); // Executa o comando
                addHistory(cmdline); // Adiciona o comando no histórico
            }    
        }
    }
    return 0;
}
