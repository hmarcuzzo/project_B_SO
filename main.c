#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "sist.h"

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

