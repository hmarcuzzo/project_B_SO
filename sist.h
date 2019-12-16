/*********************************************************************
* NOME DO ARQUIVO :        sist.h                                    *
*                                                                    *
* DESCRIÇÂO :                                                        *
*                                                                    *
*       Este programa consiste em executar funções de um shell em    *
*           um disco virtual, integrando este disco com o disco      *
*           real.                                                    *
*                                                                    *
* AUTOR :    Enzo Italiano, Henrique Marcuzzo e Matheus Batistela    *
*                                                                    *
* DATA DE CRIAÇÃO :    12/12/2019                                    *
*                                                                    *
* MODIFICAÇÕES :       15/12/2019                                    *
*                                                                    *
**********************************************************************/

#define push(sp, n) (*((sp)++) = (n))
#define pop(sp) (*--(sp))


#define DISK_NAME "disco.dsc"       // nome do disco
#define DISK_SIZE 10  * 1048576
#define BLOCK_SIZE 4096             // tamanho do bloco
#define MAX_POINTERS 300            // Número máximo de ponteiros em um único diretório

unsigned int actualDir[50];         // guarda o caminho do diretório atual
unsigned int adpCount = 0;          // a posição valida em que se encontra o actualDir

typedef enum {FREE = 0, BUSY, RESERVED, DIR, BAD} status;       // estados possíveis da FAT
typedef enum { false, true } bool;

typedef struct{
    unsigned int disk_size;
    unsigned int block_size;
    unsigned int blocks;
    unsigned int root_dir;
} SuperBlock;               // estrutura do Super Bloco

SuperBlock* disk_info;

typedef struct{
    char* name;
    unsigned int index;
    unsigned int begin;
    unsigned int size;
    unsigned int items;
    status status;
    bool first;
    unsigned int next_block;
    unsigned int pointers[MAX_POINTERS];
} Block;                // estrutura de um arquivo ou diretório

Block* FAT;

// reseta o disco com as configurações padrões iniciais
void startFileSystem(){

    FILE* disk = fopen(DISK_NAME, "wb");

    unsigned char zero = 0;

    // escreve as informações do SUper Bloco
    disk_info = malloc(sizeof(disk_info));

    disk_info->disk_size = DISK_SIZE;
    disk_info->block_size = BLOCK_SIZE;
    disk_info->blocks = (disk_info->disk_size - sizeof(disk_info))/BLOCK_SIZE;

    for (int i = 0; i < (disk_info->disk_size); i++){
        fwrite(&zero, sizeof(zero), 1, disk);
    }

    fseek(disk, 0, SEEK_SET);
    fwrite(&disk_info, sizeof(SuperBlock), 1, disk);

    // escreve a FAT no disco virtual
    FAT = malloc(disk_info->blocks*sizeof(Block));
    
    for (int i = 0; i < disk_info->blocks; i++){
        fwrite(&FAT[i], sizeof(FAT[i]), 1, disk);
    }

    // preencher as configurações da FAT padrão
    int begin_byte = ftell(disk) + 1;
    for (int i = 0; i < disk_info->blocks; i++){
        FAT[i].status = FREE;
        FAT[i].index = i;
        FAT[i].begin = begin_byte;
        FAT[i].next_block = -1;
        FAT[i].items = 0;
        begin_byte += (BLOCK_SIZE -1);        
    }


    float fblocks = (float)(disk_info->blocks*sizeof(Block))/BLOCK_SIZE ;
    int blocks_for_fat = ceil(fblocks);

    for (int i = 0; i < blocks_for_fat; i++){
        FAT[i].status = RESERVED;
    }
    

    FAT[blocks_for_fat].name = "~";
    FAT[blocks_for_fat].status = DIR;
    FAT[blocks_for_fat].first = true;
    disk_info->root_dir = FAT[blocks_for_fat].begin; 
    
    // iniciar o diretório padrão como o root
    actualDir[adpCount] = blocks_for_fat;
    adpCount++;
    

    fclose(disk);  
}

// retorna o primeiro bloco livre que encontrar
int getFreeBlock(){
    for (int i = 0; i < disk_info->blocks; i++){
        if(FAT[i].status == FREE) 
            return i;
    }
    return -1;
}

// retorna o index do arquivo passado
int getFile(char* dir_path){
    char* last_dir;
    char* token = strtok(dir_path, "/");
    last_dir = token;
    while( token != NULL ){
        last_dir = token;
        token = strtok(NULL, "/");
    }

    for (int i = 0; i < disk_info->blocks; i++){
        if((FAT[i].status == DIR || FAT[i].status == BUSY) && FAT[i].first){
            if(!strcmp(last_dir,FAT[i].name))
                return i;
        }
    }
    return -1;
}

// retorna o nome do arquivo de acordo com o path
char* getFileName(char* dir_path){
    char* last_dir;
    char* token = strtok(dir_path, "/");
    last_dir = token;
    while( token != NULL ){
        last_dir = token;
        token = strtok(NULL, "/");
    }
    return last_dir;
}

// printa os arquivos e diretórios contidos no block de diretório passado
void printDir(Block current_dir){
    Block p;
    
    if(current_dir.items == 0)
        printf("Vazio\n");
    else{

        FILE* read_ptr = fopen(DISK_NAME,"rb");

        // Block aux;
        // fseek(read_ptr,current_dir.begin,SEEK_SET);
        // printf("\nCOMECEI A LER AQUI: %ld\n",ftell(read_ptr));
        // fread(&p,sizeof(Block*)-1,1,read_ptr);
        // printf("TERMINEI DE LER AQUI: %ld\n\n",ftell(read_ptr));
        // printf("ARQUIVO %s\n",p.name);

        // for (int i = 1; i < current_dir.items ; i++){
        //     fseek(read_ptr,current_dir.begin + sizeof(Block*)*i,SEEK_SET);
        //     printf("\nCOMECEI A LER AQUI: %ld\n",ftell(read_ptr));
        //     fread(&p,sizeof(Block*)-1,1,read_ptr);            
        //     printf("TERMINEI DE LER AQUI: %ld\n\n",ftell(read_ptr));
        //     printf("ARQUIVO %s\n",p.name);
            
        // }

        for (int i = 0; i < current_dir.items; i++){
            printf("%s\n", FAT[current_dir.pointers[i]].name);
        }
        
        printf("\n");    
        fclose(read_ptr);
    }
}

// escreve um arquivo do sistema real para o disco simulado
void writeInDisk(char* file_name, int dir){
    // ponteiros de escrita e leitura
    FILE* read_ptr = fopen(file_name,"rb");
    FILE* write_ptr = fopen(DISK_NAME,"wb");

    char* name = getFileName(file_name);

    // pilha auxiliar
    int stack[10];
    int* sp = stack;

    fseek(read_ptr, 0, SEEK_END);
    unsigned int file_size = ftell(read_ptr);       // variavel que contem o tamanho total do arquivo
    fseek(read_ptr, 0, SEEK_SET);

    long int loop_size_aux = file_size;
    int nblocks = 0;
    while(1){
        loop_size_aux -= BLOCK_SIZE -1;             // a cada iteração, decrementa o tamanho de um bloco

        if(nblocks == 0){                           // quando for o primeiro bloco

            int block_index = getFreeBlock();       // recebe o indice do primeiro bloco livre

            // define as caracteristicas do bloco
            FAT[block_index].status = BUSY;
            FAT[block_index].name = name;
            FAT[block_index].first = true;
            FAT[block_index].items = 1;
            FAT[dir].pointers[FAT[dir].items] = FAT[block_index].index;
            FAT[dir].items++;

            if(loop_size_aux <0){                   // caso for negativo na primeira iteração, o arquivo é menor que o tamanho de um bloco
                FAT[block_index].size = file_size;

                char buff[FAT[block_index].size];   // buffer com o tamanho do arquivo
                    fread(&buff,FAT[block_index].size,1,read_ptr); // le o conteudo do arquivo
                    fseek(write_ptr,FAT[block_index].begin,SEEK_SET); // posiciona o ponteiro no comeco do bloco
                    fwrite(buff,FAT[block_index].size,1,write_ptr);    // escreve o conteudo do buffer no bloco

                break;
            }
            else{                               // caso for positivo, significa que o arquivo é maior que um bloco
                FAT[block_index].size = BLOCK_SIZE -1; // escreve o tamnho de um bloco completo
                FAT[block_index].next_block = getFreeBlock(); // recebe um bloco livre

                push(sp,FAT[block_index].next_block);   // empilha o proximo bloco
                nblocks++;

                char buffa[FAT[block_index].size];
                    fread(&buffa,FAT[block_index].size,1,read_ptr);
                    fseek(write_ptr,FAT[block_index].begin,SEEK_SET);
                    fwrite(buffa,FAT[block_index].size,1,write_ptr);
            }
        }
        else if(loop_size_aux > 0 && nblocks > 0){      // caso ainda houver conteudo no arquivo, e nao for o primeiro bloco

            int block_index = pop(sp); // retira da pilha o bloco empilhado pelo bloco anterior

            FAT[block_index].status = BUSY;
            FAT[block_index].name = name;
            FAT[block_index].size = BLOCK_SIZE -1;
            FAT[block_index].first = false;
            FAT[block_index].items = 1;
            FAT[block_index].next_block = getFreeBlock();

            push(sp,FAT[block_index].next_block);
            nblocks++;

            char buffb[FAT[block_index].size];
                fread(&buffb,FAT[block_index].size,1,read_ptr);
                fseek(write_ptr,FAT[block_index].begin,SEEK_SET);
                fwrite(buffb,FAT[block_index].size,1,write_ptr);
            
        }
        else if(loop_size_aux < 0 && nblocks > 0){  // caso for negativo e nao for o primeiro bloco, significa que este é o ultimo bloco

            int block_index = pop(sp);

            FAT[block_index].status = BUSY;
            FAT[block_index].name = name;
            FAT[block_index].size = BLOCK_SIZE + loop_size_aux -1; // define o tamanho restante para completar o tamanho total do arquivo
            FAT[block_index].first = false;
            FAT[block_index].items = 1;

            char buffc[FAT[block_index].size];
                fread(&buffc,FAT[block_index].size,1,read_ptr);
                fseek(write_ptr,FAT[block_index].begin,SEEK_SET);
                fwrite(buffc,FAT[block_index].size,1,write_ptr);
            break;
        }

    }


    fclose(read_ptr);
    fclose(write_ptr);
}

// le o arquivo escrito no disco simulado e grava no sistema real
void readFromDisk(char* file_name, char* newFile){
    // ponteiros para leitura e escrita
    FILE* read_ptr = fopen(DISK_NAME,"rb");
    FILE* write_ptr = fopen(newFile,"wb");
    
    // pilha auxiliar
    int stack[10];
    int* sp = stack;

    int block_index = getFile(file_name);       // indice do bloco que contem o arquivo no disco simulado

    push(sp,-1);
    push(sp,FAT[block_index].index);            // empilha o primeiro bloco
    int index;
    int a = 0;
    while (1){
        index = pop(sp);
        if(index == -1) break;                          // quando nao houver um proximo bloco, para o laço
        
        fseek(read_ptr,FAT[index].begin,SEEK_SET);      // posiciona o ponteiro no começo do bloco
        char buff[FAT[index].size];                     // cria um buffer com o conteudo do bloco

        fread(buff,1,FAT[index].size,read_ptr);         // le o bloco do disco simulado

        fwrite(buff,FAT[index].size,1,write_ptr);       // escreve no disco real
        
        if(FAT[index].next_block != -1){                // caso houver um proximo bloco, empilha
            push(sp,FAT[index].next_block);
        }
    }
    fclose(read_ptr);
    fclose(write_ptr);
    

}

// cria o diretório (new_dir) no local passado por parâmetro (dir)
bool mkdir(char* new_dir, char* dir){

    FILE* write_ptr = fopen(DISK_NAME,"wb");
    int destination_dir;
    Block* p;

    int new_dir_block = getFreeBlock();         // procura o index do primeiro bloco livre

    // coloca as informações necessárias do novo diretório na sua FAT
    FAT[new_dir_block].status = DIR;
    FAT[new_dir_block].name = new_dir;
    

    if (dir != NULL){
        char str[10000];
        strcpy(str, dir);
        destination_dir = getFile(str);     // pega o index do diretório onde se deseja criar o novo diretório
    }
    else{
        destination_dir = actualDir[(adpCount - 1)];
    }

    // if(FAT[destination_dir].items == 0){
        // printf("Entrei\n");
        // fseek(write_ptr,FAT[destination_dir].begin,SEEK_SET);
        // printf("\nCOMECEI A ESCREVER AQUI: %ld\n",ftell(write_ptr));
        // printf("Pointer: %p\n",&FAT[new_dir_block]);
    //     fwrite(FAT + new_dir_block, sizeof(Block*)-1, 1, write_ptr);
    // }
    // else{
    //     fseek(write_ptr,(FAT[destination_dir].begin + ((sizeof(Block*))*(FAT[destination_dir].items))),SEEK_SET);
        // printf("\nCOMECEI A ESCREVER AQUI: %ld\n",ftell(write_ptr));
        // printf("Pointer: %p",&FAT[new_dir_block]);
    //     fwrite(FAT + new_dir_block, sizeof(Block*)-1, 1, write_ptr);
    // }
    // printf("TERMINEI AQUI: %ld\n\n",ftell(write_ptr));


    FAT[destination_dir].pointers[FAT[destination_dir].items] = new_dir_block;          // adiciona o index do novo diretório no array de ponteiro de items do diretório onde foi criado
    FAT[destination_dir].items++;                                                       // aumenta a quantidade de itens
    

    fclose(write_ptr);

    return true;

}

// printa o caminho do diretório atual
void pwd(){

    printf("/");
    for (int i = 0; i <= (adpCount - 1); i++){
        printf("%s", FAT[actualDir[i]].name);

        if (i < (adpCount - 1))
            printf("/");
    }
    printf("\n");
}

// função que simula o cd em um shell
bool cd(char* str){
    int len = 0;
    char listToken[1000][1000];
    int flag = 0;

    unsigned int aux_actual_dir[50];
    unsigned int aux_adp_count = 0;

    if (str[0] == '/'){     // se o primeiro caracter for "/" significa que deve-se começar pelo root
        flag = 1;
        aux_actual_dir[aux_adp_count] = actualDir[aux_adp_count];
        aux_adp_count++;
    }
    else{                   // caso contrário começa pelo diretório atual
        for (int i = 0; i < adpCount; i++){
            aux_actual_dir[i] = actualDir[i];
        }
        aux_adp_count = adpCount;
    }

    // função similar a um split, criando um array de strings separadas pelo caracter "/"
    char* token = strtok(str, "/");

    while (token != NULL){
        strcpy(listToken[len], token);
        len++;

        token = strtok(NULL, "/");
    }

    if (flag == 1){
        if (len == 0){      // se o primeiro caracter for "/" e o tamanho for 0, significa que quer ir para o root
            adpCount = 1;
            return true;
        }
        else if (strcmp(listToken[0], "~") != 0){   // se o primeiro caracter for "/" e o segundo não for "~", o diretório não existe
            printf("Arquivo ou diretório inexistente!\n");
            return false;
        }
        
    }

    for (int i = flag; i < len; i++){
        if (strcmp(listToken[i], "..") == 0){       // compara se é "..", caso for volta para o diretório anterior
            if(aux_adp_count > 1){
                aux_adp_count--;
            }
        }
        else{ // se não for ".." só pode ser um diretório

            bool isValid;

            // compara se o nome do diretório passado existe no diretório atual, comparando com todos os items do diretório atual
            for (int j = 0; j < FAT[aux_actual_dir[aux_adp_count - 1]].items; j++){
                int number = aux_actual_dir[aux_adp_count - 1];
                int Pname = FAT[number].pointers[j];
                isValid = false;
                
                if (strcmp(listToken[i], FAT[Pname].name) == 0){
                    aux_actual_dir[aux_adp_count] = FAT[Pname].index;
                    aux_adp_count++;

                    isValid = true;
                    break;
                }
                
            }
            
            // se não existir retorna "false"
            if (isValid == false){
                printf("Arquivo ou diretório inexistente!\n");
                return false;
            }
            
        }
        
    }
    
    // se existir copia os auxiliares para as variáveis global e retorna "true"
    for (int i = 0; i < aux_adp_count; i++){
        actualDir[i] = aux_actual_dir[i];
    }
    adpCount = aux_adp_count;
    return true;
}

// remove arquivo ou diretório
bool removeItem(char* str, int dir){

    bool isValid;

    // compara o nome passado se existe nos items do diretório passado
    for (int j = 0; j < FAT[dir].items; j++){
        int Pname = FAT[dir].pointers[j];
        isValid = false;
        
        if (strcmp(str, FAT[Pname].name) == 0){ // se existe compara seus status
            isValid = true;
            
            if (FAT[Pname].status == RESERVED){ // se for reservado não pode fazer nada
                printf("Arquivo ou diretório reservado, não é possível de ser apagado!\n");
                return false;
            }  
            else if (FAT[Pname].status == BUSY || FAT[Pname].status == DIR){

                if (FAT[Pname].status == DIR){      // se for diretório começa recursivamente a excluir todos os itens do diretório passado, seja outros diretórios ou arquivos
                    for (int i = 0; i < FAT[Pname].items; i++){
                        removeItem(FAT[FAT[Pname].pointers[i]].name, Pname);
                    }
                }
                
                int auxPointer = Pname;
                FAT[dir].pointers[j] = FAT[dir].pointers[FAT[dir].items - 1];   // copia o último item para a posição onde um item foi excluido
                
                do{ // seta os bloco onde foi excluido como FREE
                    FAT[auxPointer].status = FREE;                              
                    auxPointer = FAT[auxPointer].next_block;                    
                } while (auxPointer != -1);      
            }
            else
                isValid == false;

            break;
        }
        
    }
    
    if (isValid == false){
        printf("Arquivo ou diretório inexistente!\n");
        return false;
    }
    else{
        FAT[dir].items--;       // diminui a quantidade de itens do diretório
        return true;
    }

}

// move diretórios ou arquivos para o destino, ambos são passado por parâmetro
bool move(char* newDir, char* item){
    // 1°a Parte: procura o diretório (newDir) onde o item será movido
    int len = 0;
    char listToken[1000][1000];
    int flag = 0;

    unsigned int aux_actual_dir[50];
    unsigned int aux_adp_count = 0;

    if (newDir[0] == '/'){
        flag = 1;
        aux_actual_dir[aux_adp_count] = actualDir[aux_adp_count];
        aux_adp_count++;
    }
    else{
        for (int i = 0; i < adpCount; i++){
            aux_actual_dir[i] = actualDir[i];
        }
        aux_adp_count = adpCount;
    }

    char* token = strtok(newDir, "/");

    while (token != NULL){
        strcpy(listToken[len], token);
        len++;

        token = strtok(NULL, "/");
    }

    if (flag == 1){
        if (strcmp(listToken[0], "~") != 0){
            printf("Arquivo ou diretório de destino inexistente!\n");
            return false;
        } 
    }

    bool isValid;

    for (int i = flag; i < len; i++){
        if (strcmp(listToken[i], "..") == 0){
            if(aux_adp_count > 1){
                aux_adp_count--;
            }
        }
        else{

            isValid = false;
            for (int j = 0; j < FAT[aux_actual_dir[aux_adp_count - 1]].items; j++){
                int number = aux_actual_dir[aux_adp_count - 1];
                int Pname = FAT[number].pointers[j];
                
                if (strcmp(listToken[i], FAT[Pname].name) == 0){                    
                    aux_actual_dir[aux_adp_count] = FAT[Pname].index;
                    aux_adp_count++;

                    isValid = true;
                    break;
                }
                
            }
            
            if (isValid == false){
                printf("Arquivo ou diretório de destino inexistente!\n");
                return false;
            }
        } 
    }

    // 2°a Parte: move o index do arquivo/diretório para o novo diretório e remove do diretório antigo
    isValid = false;
    for (int i = 0; i < FAT[actualDir[adpCount - 1]].items; i++){
        int number = actualDir[adpCount - 1];
        int Pname = FAT[number].pointers[i];
        
        if (strcmp(FAT[Pname].name, item) == 0){
            // adiciona o index do arquivo/diretório para o novo diretório
            FAT[aux_actual_dir[aux_adp_count - 1]].pointers[FAT[aux_actual_dir[aux_adp_count - 1]].items] = FAT[Pname].index;
            FAT[aux_actual_dir[aux_adp_count - 1]].items++;

            // remove o index do arquivo/diretório do diretório antigo
            FAT[actualDir[adpCount - 1]].pointers[i] = FAT[actualDir[adpCount - 1]].pointers[FAT[actualDir[adpCount - 1]].items - 1];
            FAT[actualDir[adpCount - 1]].items--;

            isValid = true;
            break;
        }
         
    }

    if (isValid == false){
        printf("Arquivo ou diretório de destino inexistente!\n");
        return false;
    }
    
    return true;
}