#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define File_Name "disco.dsc"
#define SIZE_OF_BLOCK 512
#define NUMBER_OF_BLOCKS 4096

typedef struct{
    int blockSize;
    int numberOfBlocks;
    int numberOfiNodes;
    int freeBlocks;
    int freeiNodes; 
    
} superBlock;


void startFileSystem(FILE* file){

    unsigned char zero = '0';

    superBlock firstSuperBlock;

    firstSuperBlock.blockSize = SIZE_OF_BLOCK;
    firstSuperBlock.numberOfBlocks = NUMBER_OF_BLOCKS;
    firstSuperBlock.numberOfiNodes = 0;


    for (int i = 0; i < (SIZE_OF_BLOCK * NUMBER_OF_BLOCKS); i++){
        fwrite(&zero, sizeof(zero), 1, file);
    }
    
}


void main(int argc, char* argv[]){

    FILE* file;

    file = fopen(File_Name, "rb");          // tenta ler o arquivo
    if (file == 0){                         // se não existir cria o arquivo
        file = fopen(File_Name, "wb");
        startFileSystem(file);
    }

    


}