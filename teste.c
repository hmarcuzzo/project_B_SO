 
    int freedBlock = getFreeBlock();
    printf("Free Block: %d\n",freedBlock);
    
    FAT[freedBlock].status = DIR;
    FAT[freedBlock].name = "Diretório 1";
    printDir(FAT[freedBlock]);

    FAT[33].status = BUSY;
    FAT[33].name = "Arquivo 1";

    FILE* write_ptr = fopen(DISK_NAME,"wb");
    fseek(write_ptr,FAT[32].begin,SEEK_SET);
    fwrite(&FAT[33], sizeof(Block*), 1, write_ptr);
    FAT[32].items += 1;

    fclose(write_ptr);

    printDir(FAT[freedBlock]);
