#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define push(sp, n) (*((sp)++) = (n))
#define pop(sp) (*--(sp))


#define DISK_NAME "disco.dsc"
#define DISK_SIZE 10  * 1048576
#define BLOCK_SIZE 4096
#define MAX_POINTERS 300

unsigned int actualDir[50];
unsigned int adpCount = 0;

typedef enum {FREE = 0, BUSY, RESERVED, DIR, BAD} status;
typedef enum { false, true } bool;

typedef struct{
    unsigned int disk_size;
    unsigned int block_size;
    unsigned int blocks;
    unsigned int root_dir;
} SuperBlock;

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
} Block;

Block* FAT;
 
void startFileSystem(){

    FILE* disk = fopen(DISK_NAME, "wb");

    unsigned char zero = 0;

    disk_info = malloc(sizeof(disk_info));

    disk_info->disk_size = DISK_SIZE;
    disk_info->block_size = BLOCK_SIZE;
    disk_info->blocks = (disk_info->disk_size - sizeof(disk_info))/BLOCK_SIZE;

    for (int i = 0; i < (disk_info->disk_size); i++){
        fwrite(&zero, sizeof(zero), 1, disk);
    }

    fseek(disk, 0, SEEK_SET);
    fwrite(&disk_info, sizeof(SuperBlock), 1, disk);

    FAT = malloc(disk_info->blocks*sizeof(Block));
    
    for (int i = 0; i < disk_info->blocks; i++){
        fwrite(&FAT[i], sizeof(FAT[i]), 1, disk);
    }

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
    printf("Blocks for fat: %ld\n",blocks_for_fat);

    for (int i = 0; i < blocks_for_fat; i++){
        FAT[i].status = RESERVED;
    }
    

    FAT[blocks_for_fat].name = "~";
    FAT[blocks_for_fat].status = DIR;
    FAT[blocks_for_fat].first = true;
    disk_info->root_dir = FAT[blocks_for_fat].begin; 
    
    actualDir[adpCount] = blocks_for_fat;
    adpCount++;
    

    fclose(disk);  
}

int getFreeBlock(){
    for (int i = 0; i < disk_info->blocks; i++){
        if(FAT[i].status == FREE) 
            return i;
    }
    return -1;
}

int getFile(char* dir_path){
    char* stack[100];
    char** sp = stack;
    char* token = strtok(dir_path, "/");
    char* last_dir;
    while( token != NULL ) {
        push(sp,token);

    // printf("%s\n",token);
        token = strtok(NULL, "/");
    }
    last_dir = pop(sp);
    // printf("%s\n",last_dir);
    for (int i = 0; i < disk_info->blocks; i++){
        if((FAT[i].status == DIR || FAT[i].status == BUSY) && FAT[i].first){
            if(!strcmp(last_dir,FAT[i].name))
                return i;
        }
    }
    return -1;
}

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


void writeInDisk(char* file_name, int dir){
    FILE* read_ptr = fopen(file_name,"rb");
    FILE* write_ptr = fopen(DISK_NAME,"wb");
    int stack[10];
    int* sp = stack;

    fseek(read_ptr, 0, SEEK_END);
    unsigned int file_size = ftell(read_ptr);
    fseek(read_ptr, 0, SEEK_SET);

    long int loop_size_aux = file_size;
    int nblocks = 0;
    while(1){
        loop_size_aux -= BLOCK_SIZE -1;

        if(nblocks == 0){

            int block_index = getFreeBlock();

            FAT[block_index].status = BUSY;
            FAT[block_index].name = file_name;
            FAT[block_index].first = true;
            FAT[block_index].items = 1;
            FAT[dir].pointers[FAT[dir].items] = FAT[block_index].index;
            FAT[dir].items++;

            // printf("%d - Entrei aqui com índice: %d\n",nblocks,block_index);

            if(loop_size_aux <0){
                FAT[block_index].size = file_size;

                char buff[FAT[block_index].size];
                    fread(&buff,FAT[block_index].size,1,read_ptr);
                    fseek(write_ptr,FAT[block_index].begin,SEEK_SET);
                    fwrite(buff,FAT[block_index].size,1,write_ptr);

                // printf("E tamanho: %d\n",FAT[block_index].size);
                break;
            }
            else{
                FAT[block_index].size = BLOCK_SIZE -1;
                FAT[block_index].next_block = getFreeBlock();

                push(sp,FAT[block_index].next_block);
                nblocks++;

                char buffa[FAT[block_index].size];
                    fread(&buffa,FAT[block_index].size,1,read_ptr);
                    fseek(write_ptr,FAT[block_index].begin,SEEK_SET);
                    fwrite(buffa,FAT[block_index].size,1,write_ptr);

                // printf("E tamanho: %d\n",FAT[block_index].size);
                // printf("O proximo bloco é: %d\n ",FAT[block_index].next_block);
                // printf("BUFFER\n");
                // for (int i = 0; i < FAT[block_index].size ; i++)
                // {
                //     printf("%c",buffa[i]);
                // }
                // printf("\n");
            }
        }
        else if(loop_size_aux > 0 && nblocks > 0){

            int block_index = pop(sp);

            FAT[block_index].status = BUSY;
            FAT[block_index].name = file_name;
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

            // printf("%d - Entrei aqui com índice: %d\n",nblocks,block_index);
            // printf("E tamanho: %d\n",FAT[block_index].size);
            // printf("O proximo bloco é: %d\n ",FAT[block_index].next_block);
            // printf("BUFFER\n");
            // for (int i = 0; i < FAT[block_index].size ; i++)
            // {
            //     printf("%c",buffb[i]);
            // }
            // printf("\n");
            
        }
        else if(loop_size_aux < 0 && nblocks > 0){

            int block_index = pop(sp);

            FAT[block_index].status = BUSY;
            FAT[block_index].name = file_name;
            FAT[block_index].size = BLOCK_SIZE + loop_size_aux -1;
            FAT[block_index].first = false;
            FAT[block_index].items = 1;

            char buffc[FAT[block_index].size];
                fread(&buffc,FAT[block_index].size,1,read_ptr);
                fseek(write_ptr,FAT[block_index].begin,SEEK_SET);
                fwrite(buffc,FAT[block_index].size,1,write_ptr);

            // printf("%d - Entrei aqui com índice: %d\n",nblocks,block_index);
            // printf("E tamanho: %d\n",FAT[block_index].size);
            // printf("BUFFER\n");
            // for (int i = 0; i < FAT[block_index].size ; i++)
            // {
            //     printf("%c",buffc[i]);
            // }
            // printf("\n");
            break;
        }

    }


    fclose(read_ptr);
    fclose(write_ptr);
}

void readFromDisk(char* file_name){
    FILE* read_ptr = fopen(DISK_NAME,"rb");
    FILE* teste = fopen("novoteste1.txt","wb");
 
    int stack[10];
    int* sp = stack;

    int block_index = getFile(file_name);

    push(sp,-1);
    push(sp,FAT[block_index].index);
    int index;
    int a = 0;
    while (1){
        index = pop(sp);
        if(index == -1) break;
        // printf("Indice: %d\n",index);
        
        fseek(read_ptr,FAT[index].begin,SEEK_SET);
        // printf("\n\nVOU LER: %ld\n\n",ftell(read_ptr));
        char b[FAT[index].size];

        fread(b,1,FAT[index].size,read_ptr);
        // printf("\n\nLI: %ld\n\n",ftell(read_ptr));

        // for (int i = 0; i< FAT[index].size ; i++){
        //     printf("%c",b[i]);
        // }
        // printf("\n");

        fwrite(b,FAT[index].size,1,teste);
        
        if(FAT[index].next_block != -1){
            push(sp,FAT[index].next_block);
        }
        a+=FAT[index].size+1;
    }
    // printf("tamanho: %ld\n",ftell(teste));
    fclose(read_ptr);
    fclose(teste);
    

}

bool mkdir(char* new_dir, char* dir){

    FILE* write_ptr = fopen(DISK_NAME,"wb");
    int destination_dir;
    Block* p;

    int new_dir_block = getFreeBlock();
    printf("New dir: %d\n",new_dir_block);

    FAT[new_dir_block].status = DIR;
    FAT[new_dir_block].name = new_dir;
    

    if (dir != NULL){
        char str[10000];
        strcpy(str, dir);
        destination_dir = getFile(str);
    }
    else{
        destination_dir = actualDir[(adpCount - 1)];
    }

    
    // printf("destination_dir: %d\n", destination_dir);
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

    FAT[destination_dir].pointers[FAT[destination_dir].items] = new_dir_block;
    FAT[destination_dir].items++;
    
    printf("destination_dir: %d\n", destination_dir);

    fclose(write_ptr);

    return true;

}

void pwd(){

    printf("/");
    for (int i = 0; i <= (adpCount - 1); i++){
        printf("%s", FAT[actualDir[i]].name);

        if (i < (adpCount - 1))
            printf("/");
    }
    printf("\n");
}

bool cd(char* str){
    int len = 0;
    char listToken[1000][1000];
    int flag = 0;

    unsigned int aux_actual_dir[50];
    unsigned int aux_adp_count = 0;

    if (str[0] == '/'){
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

    char* token = strtok(str, "/");

    while (token != NULL){
        strcpy(listToken[len], token);
        len++;

        token = strtok(NULL, "/");
    }

    if (flag == 1){
        if (len == 0){
            adpCount = 1;
            return true;
        }
        else if (strcmp(listToken[0], "~") != 0){
            printf("Arquivo ou diretório inexistente!\n");
            return false;
        }
        
    }

    for (int i = flag; i < len; i++){
        if (strcmp(listToken[i], "..") == 0){
            if(aux_adp_count > 1){
                aux_adp_count--;
            }
        }
        else{

            bool isValid;

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
            
            if (isValid == false){
                printf("Arquivo ou diretório inexistente!\n");
                return false;
            }
            
        }
        
    }
    

    for (int i = 0; i < aux_adp_count; i++){
        // printf("aux_actual_dir[i]: %d\n", aux_actual_dir[i]);
        actualDir[i] = aux_actual_dir[i];
    }
    adpCount = aux_adp_count;
    return true;
}

bool removeItem(char* str, int dir){

    bool isValid;

    for (int j = 0; j < FAT[dir].items; j++){
        int Pname = FAT[dir].pointers[j];
        isValid = false;
        
        if (strcmp(str, FAT[Pname].name) == 0){
            isValid = true;
            
            if (FAT[Pname].status == RESERVED){
                printf("Arquivo ou diretório reservado, não é possível de ser apagado!\n");
                return false;
            }  
            else if (FAT[Pname].status == BUSY || FAT[Pname].status == DIR){

                if (FAT[Pname].status == DIR){
                    for (int i = 0; i < FAT[Pname].items; i++){
                        removeItem(FAT[FAT[Pname].pointers[i]].name, Pname);
                    }
                }
                
                int auxPointer = Pname;
                FAT[dir].pointers[j] = FAT[dir].pointers[FAT[dir].items - 1];
                
                do{
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
        FAT[dir].items--;
        return true;
    }

}

bool copy(char* newDir, char* item){
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


    isValid = false;
    for (int i = 0; i < FAT[actualDir[adpCount - 1]].items; i++){
        int number = actualDir[adpCount - 1];
        int Pname = FAT[number].pointers[i];
        
        if (strcmp(FAT[Pname].name, item) == 0){
            FAT[aux_actual_dir[aux_adp_count - 1]].pointers[FAT[aux_actual_dir[aux_adp_count - 1]].items] = FAT[Pname].index;
            
        }
         
    }

    if (isValid == false){
        printf("Arquivo ou diretório de destino inexistente!\n");
        return false;
    }
    
    return true;
}

int main(){

    char strTeste[15];

    startFileSystem();

    printf("Index atual: %d\n", actualDir[adpCount - 1]);
    mkdir("teste_01", NULL);

    strcpy(strTeste, "/~/teste_01");
    cd(strTeste);

    int dir = actualDir[adpCount - 1];
    writeInDisk("teste1.txt", dir);
    printf("Dir: %d\n",getFile("~"));
     for (int i = 0; i < disk_info->blocks; i++){
        if(FAT[i].status == DIR){
            if(!strcmp("~",FAT[i].name))
                printf("Existo no bloco: %d\n",FAT[i].index);
        }
    }
    
    readFromDisk("teste1.txt");
    printf("Free: %d\n",getFreeBlock());


    // int freedBlock = getFreeBlock();
    // printf("Free Block: %d\n",freedBlock);
    
    // FAT[freedBlock].status = DIR;
    // FAT[freedBlock].name = "Diretório 1";
    // printDir(FAT[freedBlock]);

    FAT[33].status = BUSY;
    FAT[33].name = "Arquivo 1";
    FAT[33].first = true;
    printf("Tamanho bloco: %ld\n",sizeof(FAT[33]));

    // FILE* write_ptr = fopen(DISK_NAME,"wb");
    // fseek(write_ptr,FAT[32].begin,SEEK_SET);
    // fwrite(&FAT[33], sizeof(Block*), 1, write_ptr);
    // FAT[32].items += 1;

    // fclose(write_ptr);

    // printDir(FAT[freedBlock]);


    // actualDir[adpCount] = 32;
    // adpCount++;
    // char oi[100] = "dir/Arquivo 1";
    // char stro[] = "dir/Arquivo 1";
    // printf("File: %d\n",getFile(oi));

    

    mkdir("teste_02", "~");
    mkdir("teste_03", "~");

    printf("Dir: %d\n", actualDir[adpCount - 1]);
    pwd();
    printDir(FAT[actualDir[adpCount - 1]]);

    strcpy(strTeste, "/~");
    copy(strTeste, "teste1.txt");

    cd("..");
    printDir(FAT[actualDir[adpCount - 1]]);

    // strcpy(strTeste, "teste_01");
    // removeItem(strTeste, actualDir[adpCount - 1]);
    // printDir(FAT[actualDir[adpCount - 1]]);

    pwd();

    return 0;
}

