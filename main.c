#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define push(sp, n) (*((sp)++) = (n))
#define pop(sp) (*--(sp))

#define DISK_NAME "disco.dsc"
#define DISK_SIZE 10  * 1048576
#define BLOCK_SIZE 4096

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
} Block;

Block* FAT;
 
void startFileSystem(){

    FILE* disk = fopen(DISK_NAME, "wb+");

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

    for (int i = 0; i < blocks_for_fat; i++){
        FAT[i].status = RESERVED;
    }
    

    FAT[blocks_for_fat].name = "~";
    FAT[blocks_for_fat].status = DIR;
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

    printf("%s\n",token);
        token = strtok(NULL, "/");
    }
    last_dir = pop(sp);
    printf("%s\n",last_dir);
    for (int i = 0; i < disk_info->blocks; i++){
        if((FAT[i].status == DIR || FAT[i].status == BUSY) && FAT[i].first){
            if(!strcmp(last_dir,FAT[i].name))
                return i;
        }
    }
    return -1;
}

void printDir(Block current_dir){
    if(current_dir.items == 0)
        printf("Vazio\n");
    else{
        Block aux;
        FILE* disk = fopen(DISK_NAME,"rb");
        fseek(disk,current_dir.begin,SEEK_SET);
        for (int i = 1; i <= current_dir.items; i++){
            fread(&aux,sizeof(Block*),1,disk);
            printf("%s\t",aux.name);
            fseek(disk, current_dir.begin + (sizeof(Block*)*i) +1  ,SEEK_SET);
        }
        printf("\n");    
        fclose(disk);
    }
}


void writeInDisk(char* file_name){
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

bool mkdir(char* dir, char* new_dir){
    
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

int main(){

    startFileSystem();

    writeInDisk("teste1.txt");
    char str[] = "eae/oi/teste.txt";
    printf("Dir: %d\n",getFile("~"));
     for (int i = 0; i < disk_info->blocks; i++){
        if(FAT[i].status == DIR){
            if(!strcmp("~",FAT[i].name))
                printf("Existo no bloco: %d\n",FAT[i].index);
        }
    }
    
    readFromDisk("teste1.txt");
    printf("Free: %d\n",getFreeBlock());
    
    pwd();
    return 0;
}

