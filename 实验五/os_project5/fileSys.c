//
// Created by TC on 2020/6/5.
//

#include "fileSys.h"

//虚拟磁盘空间保存文件名
char *SYSNAME = "tsfilesys";

void initSys() {
    /*
     * 文件系统启动函数，读取文件系统内容
     * 存在SYSNME则打开，不存在则调用my_format创建文件系统
     * 将root目录加入打开文件表
     */
    //分配虚拟磁盘内存
    myhard = (unsigned char *) malloc(SIZE);
    FILE *file;
    //打开文件系统文件或创建
    if ((file = fopen(SYSNAME, "r")) != NULL) {
        fread(buffer, SIZE, 1, file);
        fclose(file);
        //判断魔数是否正确
        if (memcmp(buffer, "Irisfsys", 8) == 0) {
            memcpy(myhard, buffer, SIZE);
            printf("File System loaded successfully\n");
        } else {
            printf("Invalid File System, rebuilding...\n");
            my_format();
            memcpy(buffer, myhard, SIZE);
        }
    } else {
        printf("File System not created, building...\n");
        my_format();
        memcpy(buffer, myhard, SIZE);
    }

    //初始化root目录
    fcb *root = (fcb *) (myhard + 5 * BLOCKSIZE);
    //将root写入文件打开表filelist
    _copy(0, root, 0);  //直接将root内容复制到filelist[0]
    filelist[0].dirno = 5;
    filelist[0].diroff = 0;
    strcpy(filelist[0].dir, "/root/");
    filelist[0].pos = 0;
    filelist[0].fcbstate = 0;
    filelist[0].topenfile = 1;

    startb = ((sysblock *) myhard)->startblock;   //数据区开始位置
    currfd = 0;
}

//退出文件系统
void my_exitsys() {
    //依次关闭已打开文件，写入SYSNAME文件中（保存）
    while (currfd) {
        my_close(currfd);
    }
    FILE *fp = fopen(SYSNAME, "w");
    fwrite(myhard, SIZE, 1, fp);
    fclose(fp);
}

//磁盘格式化，初始虚拟磁盘，FAT表和根目录
void my_format() {
    //初始化两个fat表
    fat *fat1 = (fat *) (myhard + BLOCKSIZE);
    fat *fat2 = (fat *) (myhard + BLOCKSIZE * 3);
    for (int i = 0; i < 6; i++) {
        fat1[i].id = END;
        fat2[i].id = END;
    }
    for (int i = 6; i < 1000; i++) {
        fat1[i].id = FREE;
        fat2[i].id = FREE;
    }

    //初始化引导块
    sysblock *boot = (sysblock *) myhard;
    strcpy(boot->magic_number, "Irisfsys");
    strcpy(boot->information, "Fat file system, block size is 1KB, number is 1000");
    boot->root = 5;
    boot->startblock = myhard + BLOCKSIZE * (5 + ROOTBLOCKNUM);

    //初始化根目录
    fcb *root = (fcb *) (myhard + BLOCKSIZE * 5);
    strcpy(root->filename, ".");
    strcpy(root->exname, "di");
    root->attribute = ATTR_DIR;  //目录文件

    time_t rawTime = time(NULL);
    struct tm *time = localtime(&rawTime);
    root->time = time->tm_hour * 2048 + time->tm_min * 32 + time->tm_sec / 2;
    root->date = (time->tm_year - 100) * 512 + (time->tm_mon + 1) * 32 + (time->tm_mday);
    root->first = 5;
    root->free = 1;
    root->length = sizeof(fcb);

    //初始化特殊目录..
    fcb *root2 = root + 1;
    memcpy(root2, root, sizeof(fcb));
    strcpy(root2->filename, "..");

    //初始化剩余fcb块
    for (int i = 2; i < (int) (BLOCKSIZE / sizeof(fcb)); i++) {
        root++;
        strcpy(root2->filename, "");
        root2->free = 0;
    }

    FILE *fp = fopen(SYSNAME, "w");
    fwrite(myhard, SIZE, 1, fp);
    fclose(fp);
}

void my_mkdir(char *dirname){
    char *fname = strtok(dirname, ".");
    char *exname = strtok(NULL, ".");
    if(exname != NULL) {
        printf("folder can not include extension name\n");
        return;
    }

    //读取父目录信息到内存
    char text[MAXREADSIZE];
    filelist[currfd].pos = 0;
    int filelen = do_read(currfd, filelist[currfd].length, text);
    fcb *fcbptr = (fcb*)text;

    //查找是否存在这个名字的文件夹
    for(int i=0; i<(int)(filelen / sizeof(fcb)); i++){
        if(strcmp(dirname, fcbptr[i].filename) == 0 && fcbptr->attribute
         == ATTR_DIR){
            printf("folder already exist\n");
            return;
        }
    }

    //申请一个打开目录表项
    int fd = get_free_filelist();
    if(fd == -1){
        printf("File number has reached the limit\n");
        return;
    }
    //申请一个磁盘块
    unsigned short int block_num = get_free_block();
    if(block_num == END){
        printf("Disk is already full\n");
        filelist[fd].topenfile = 0;
        return;
    }
    //更新fat表
    fat *fat1 = (fat *) (myhard + BLOCKSIZE);
    fat *fat2 = (fat *) (myhard + BLOCKSIZE * 3);
    fat1[block_num].id = END;
    fat2[block_num].id = END;

    //在父目录找一个空的fcb分配给该目录
    int i=0;
    for(i=0; i<(int)(filelen / sizeof(fcb)); i++){
        if(fcbptr[i].free == 0){
            break;
        }
    }
    filelist[currfd].pos = i * sizeof(fcb);
    filelist[currfd].fcbstate = 1;
    //初始化该fcb
    fcb *fcbtmp = (fcb*)malloc(sizeof(fcb));
    fcbtmp->attribute = ATTR_DIR;
    time_t rawtime = time(NULL);
    struct tm* time = localtime(&rawtime);
    fcbtmp->date = (time->tm_year - 100) * 512 + (time->tm_mon + 1) * 32 + (time->tm_mday);
    fcbtmp->time = (time->tm_hour) * 2048 + (time->tm_min) * 32 + (time->tm_sec) / 2;
    strcpy(fcbtmp->filename, dirname);
    strcpy(fcbtmp->exname, "di");
    fcbtmp->first = block_num;
    fcbtmp->length = 2 * sizeof(fcb);
    fcbtmp->free = 1;
    do_write(currfd, (char*)fcbtmp, sizeof(fcb), 2);

    //设置打开文件表项
    _copy(fd, fcbtmp, 0);
    filelist[fd].pos = 0;
    filelist[fd].dirno = filelist[currfd].first;
    filelist[fd].diroff = i;
    strcpy(filelist[fd].exname, "di");
    strcpy(filelist[fd].filename, dirname);
    filelist[fd].fcbstate = 0;
    filelist[fd].topenfile = 1;
    strcat(strcat(strcpy(filelist[fd].dir, (char *)(filelist[currfd].dir)), dirname), "/");

    //设置.和..目录
    strcpy(fcbtmp->filename, ".");
    do_write(fd, (char*)fcbtmp, sizeof(fcb), 2);
    strcpy(fcbtmp->filename, "..");
    fcbtmp->first = filelist[currfd].first;
    fcbtmp->length = filelist[currfd].length;
    fcbtmp->date = filelist[currfd].date;
    fcbtmp->time = filelist[currfd].time;
    do_write(fd, (char *)fcbtmp, sizeof(fcb), 2);

    my_close(fd);
    free(fcbtmp);

    //更新父目录的fcb
    fcbptr = (fcb*)text;
    fcbptr->length = filelist[currfd].length;
    filelist[currfd].pos = 0;
    do_write(currfd, (char*)fcbptr, sizeof(fcb), 2);
    filelist[currfd].fcbstate = 1;
}

void my_rmdir(char *dirname){
    if(strcmp(dirname, ".") == 0||strcmp(dirname, "..") == 0){
        printf("sorry, you can't remove special folder");
        return ;
    }

    //读当前目录文件内容到内存
    char buf[MAXREADSIZE];
    filelist[currfd].pos = 0;
    do_read(currfd, filelist[currfd].length, buf);

    //遍历查找要删除的目录
    fcb *fcbptr = (fcb*)buf;
    int flag = -1;
    for(int i=0; i<(int)(filelist[currfd].length/ sizeof(fcb)); i++, fcbptr++){
        if(fcbptr->free == 0)
            continue;
        if(strcmp(fcbptr->filename, dirname) == 0&& fcbptr->attribute==ATTR_DIR){
            flag = i;
            break;;
        }
    }
    if(flag == -1){
        printf("The folder not exist\n");
        return;
    }
    //无法删除非空目录
    if(fcbptr->length > 2 * sizeof(fcb)){
        printf("Can not remove folder not empty\n");
        return;
    }

    //更新fat表
    int block_num = fcbptr->first;
    fat *fat1 = (fat*)(myhard + BLOCKSIZE);
    int rm_num = 0;
    while(1){
        rm_num = fat1[block_num].id;
        fat1[rm_num].id = FREE;
        if(rm_num != END){
            block_num = rm_num;
        }else{
            break;
        }
    }
    fat1 = (fat*)(myhard + BLOCKSIZE);
    fat* fat2 = (fat*)(myhard + BLOCKSIZE * 3);
    memcpy(fat2, fat1, BLOCKSIZE * 2);

    //清空fcb
    fcbptr->date = 0;
    fcbptr->time = 0;
    fcbptr->exname[0] = '\0';
    fcbptr->filename[0] = '\0';
    fcbptr->first = 0;
    fcbptr->free = 0;
    fcbptr->length = 0;

    //修改打开文件表
    filelist[currfd].pos = flag * sizeof(fcb);
    do_write(currfd, (char*)fcbptr, sizeof(fcb), 2);

    //如果删除的是最后一个fcb，循环删除前面所有空fcb
    int lognum = flag;
    if ((lognum + 1) * sizeof(fcb) == filelist[currfd].length) {
        filelist[currfd].length -= sizeof(fcb);
        lognum--;
        fcbptr = (fcb *)buf + lognum;
        while (fcbptr->free == 0) {
            fcbptr--;
            filelist[currfd].length -= sizeof(fcb);
        }
    }

    //更新父目录fcb
    fcbptr = (fcb*)buf;
    fcbptr->length = filelist[currfd].length;
    filelist[currfd].pos = 0;
    do_write(currfd, (char*)fcbptr, sizeof(fcb), 2);
    filelist[currfd].fcbstate = 1;
}

int my_cd(char *dirname){
    if(filelist[currfd].attribute == ATTR_FILE){
        printf("Can not change folder in file, please close file first");
        return -1;
    }

    //读当前目录信息
    char *buf = (char*)malloc(MAXREADSIZE);
    filelist[currfd].pos = 0;
    do_read(currfd, filelist[currfd].length, buf);

    //查找目标fcb
    fcb *fcbptr = (fcb*)buf;
    int flag = -1;
    for(int i=0; i<(int)(filelist[currfd].length / sizeof(fcb)); i++, fcbptr++){
        if(strcmp(fcbptr->filename, dirname) == 0 && fcbptr->attribute==ATTR_DIR){
            flag = i;
            break;
        }
    }
    if(flag == -1){
        printf("no %s folder\n", dirname);
        return -1;
    }else{
        if(strcmp(fcbptr->filename, ".") == 0){
            return 0;
        }else if(strcmp(fcbptr->filename, "..") == 0){
            if(currfd == 0){
                return 0;
            }else{
                currfd = my_close(currfd);
                return 0;
            }
        }else{
            //申请一个打开文件表项，将找到的目录信息写入并设置成当前目录
            int fd = get_free_filelist();
            if(fd == -1){
                return -1;
            }
            _copy(fd, fcbptr, 0);
            filelist[fd].fcbstate = 0;
            filelist[fd].topenfile = 1;
            filelist[fd].dirno = filelist[currfd].first;
            filelist[fd].diroff = flag;
            strcat(strcat(strcpy(filelist[fd].dir, (char*)(filelist[currfd].dir)), dirname), "/");
            currfd = fd;
        }
    }
}

void my_ls() {
    //判断是否是目录
    if (filelist[currfd].attribute == ATTR_FILE) {
        printf("Can not view file list in file\n");
        return;
    }

    //读取当前目录文件信息，载入内存
    char buf[MAXREADSIZE];
    filelist[currfd].pos = 0;
    do_read(currfd, filelist[currfd].length, buf);

    fcb *fcbptr = (fcb *) buf;
    printf("%-6s %-12s %-12s %-7s %s\n", "type", "filename", "date", "time", "size");
    for (int i = 0; i < (int) (filelist[currfd].length / sizeof(fcb)); i++, fcbptr++) {
        //遍历当前目录fcb
        if (fcbptr->free == 1) {
            if (fcbptr->attribute == ATTR_DIR) {
                printf(" <DIR>  %-12s %04d/%02d/%02d   %02d:%02d\n",
                       fcbptr->filename,
                       (fcbptr->date >> 9) + 2000,
                       (fcbptr->date >> 5) & 0x000f,
                       (fcbptr->date) & 0x001f,
                       (fcbptr->time >> 11),
                       (fcbptr->time >> 5) & 0x003f);
            } else {
                printf("<FILE>  %-12s %04d/%02d/%02d   %02d:%02d   %dB\n",
                       fcbptr->filename,
                       (fcbptr->date >> 9) + 2000,
                       (fcbptr->date >> 5) & 0x000f,
                       (fcbptr->date) & 0x001f,
                       (fcbptr->time >> 11),
                       (fcbptr->time >> 5) & 0x003f,
                       fcbptr->length);
            }
        }
    }
}

int my_create(char *filename){
    if(strcmp(filename, "") == 0){
        printf("Please input valid file name");
        return -1;
    }
    if(filelist[currfd].attribute == ATTR_FILE){
        printf("Can not do create file in a file");
        return -1;
    }

    //读当前目录文件表项到内存
    filelist[currfd].pos = 0;
    char buf[MAXREADSIZE];
    do_read(currfd, filelist[currfd].length, buf);

    //检测是否有重名
    fcb *fcbptr = (fcb*)buf;
    for(int i=0; i<(int)(filelist[currfd].length/ sizeof(fcb)); i++, fcbptr++){
        if(fcbptr->free == 0){
            continue;
        }
        if(strcmp(fcbptr->filename, filename) == 0){
            printf("This file name: %s already exist\n", filename);
            return -1;
        }
    }

    //申请空的fcb
    fcbptr = (fcb*)buf;
    int i=0;
    for(i=0; i<(int)(filelist[currfd].length/ sizeof(fcb)); i++, fcbptr++) {
        if (fcbptr->free == 0)
            break;
    }
    int new_fcb_index = i;

    //申请空闲盘块，更新FAT表
    int block_num = get_free_block();
    if(block_num == -1)
        return -1;
    fat* fat1 = (fat*)(myhard + BLOCKSIZE);
    fat* fat2 = (fat*)(myhard + BLOCKSIZE * 3);
    fat1[block_num].id = END;
    memcpy(fat2, fat1, BLOCKSIZE*2);

    //修改当前打开文件表项信息
    strcpy(fcbptr->filename, filename);
    time_t rawtime = time(NULL);
    struct tm* time = localtime(&rawtime);
    fcbptr->date = (time->tm_year - 100) * 512 + (time->tm_mon + 1) * 32 + (time->tm_mday);
    fcbptr->time = (time->tm_hour) * 2048 + (time->tm_min) * 32 + (time->tm_sec) / 2;
    fcbptr->first = block_num;
    fcbptr->free = 1;
    fcbptr->attribute = ATTR_FILE;
    fcbptr->length = 0;
    filelist[currfd].pos = new_fcb_index * sizeof(fcb);
    do_write(currfd, (char*)fcbptr, sizeof(fcb), 2);

    //修改父目录fcb,即当前打开目录的fcb
    fcbptr = (fcb*)buf;
    fcbptr->length = filelist[currfd].length;
    filelist[currfd].pos = 0;
    do_write(currfd, (char*)fcbptr, sizeof(fcb), 2);
    filelist[currfd].fcbstate = 1;
}

void my_rm(char *filename){
    //读当前打开目录文件表项到内存
    char buf[MAXREADSIZE];
    filelist[currfd].pos = 0;
    do_read(currfd, filelist[currfd].length, buf);

    //查询文件
    int flag = -1;
    fcb *fcbptr = (fcb*)buf;
    for(int i=0; i<(int)(filelist[currfd].length/ sizeof(fcb)); i++, fcbptr++){
        if(strcmp(fcbptr->filename, filename) == 0 && fcbptr->attribute == ATTR_FILE){
            flag = i;
            break;
        }
    }
    if(flag == -1){
        printf("no %s folder\n", filename);
        return;
    }

    //更新fat表
    int block_num = fcbptr->first;
    fat *fat1 = (fat*)(myhard + BLOCKSIZE);
    int rm_num = 0;
    while(1){
        //循环把文件占据的所有block都删除，即FAT置为FREE
        rm_num = fat1[block_num].id;
        fat1[block_num].id = FREE;
        if(rm_num != END){
            block_num = rm_num;
        }else{
            break;
        }
    }
    fat* fat2 = (fat*)(myhard + BLOCKSIZE * 3);
    memcpy(fat2, fat1, BLOCKSIZE * 2);

    //清空fcb
    fcbptr->date = 0;
    fcbptr->time = 0;
    fcbptr->exname[0] = '\0';
    fcbptr->filename[0] = '\0';
    fcbptr->first = 0;
    fcbptr->free = 0;
    fcbptr->length = 0;
    filelist[currfd].pos = flag * sizeof(fcb);
    do_write(currfd, (char*)fcbptr, sizeof(fcb), 2);


    //循环删除所有空fcb
    int lognum = flag;
    if ((lognum + 1) * sizeof(fcb) == filelist[currfd].length) {
        filelist[currfd].length -= sizeof(fcb);
        lognum--;
        fcbptr = (fcb *)buf + lognum;
        while (fcbptr->free == 0) {
            fcbptr--;
            filelist[currfd].length -= sizeof(fcb);
        }
    }

    //修改父目录的fcb
    fcbptr = (fcb*)buf;
    fcbptr->length = filelist[currfd].length;
    filelist[currfd].pos = 0;
    do_write(currfd, (char*)fcbptr, sizeof(fcb), 2);

    filelist[currfd].fcbstate = 1;
}

int my_open(char *filename){
    //读当前打开目录文件表项到内存
    char buf[MAXREADSIZE];
    filelist[currfd].pos = 0;
    do_read(currfd, filelist[currfd].length, buf);

    //遍历查找filename文件
    int flag = -1;
    fcb *fcbptr = (fcb*)buf;
    for(int i=0; i<(int)(filelist[currfd].length / sizeof(fcb)); i++, fcbptr++){
        if(strcmp(fcbptr->filename, filename)==0 && fcbptr->attribute==ATTR_FILE){
            flag = i;
            break;
        }
    }
    if(flag == -1){
        printf("no %s file\n", filename);
        return -1;
    }

    //申请新的打开目录项并初始化
    int fd = get_free_filelist();
    if(fd == -1){
        printf("Can not open the file because of limit of 10");
        return -1;
    }
    _copy(fd, fcbptr, 0);
    strcat(strcpy(filelist[fd].dir, (char*)(filelist[currfd].dir)), filename);
    filelist[fd].dirno = filelist[currfd].first;
    filelist[fd].diroff = flag;
    filelist[fd].topenfile = 1;
    filelist[fd].fcbstate = 0;
    currfd = fd;
    return 1;
}

int my_close(int fd) {
    if (fd > MAXOPENFILE || fd < 0) {
        printf("Opened file list wrong\n");
        return -1;
    }

    //找到父目录
    int father_fd = -1;
    for (int i = 0; i < MAXOPENFILE; i++) {
        if (filelist[i].first == filelist[fd].dirno) {
            father_fd = i;
            break;
        }
    }
    if (father_fd == -1) {
        printf("no parent directory");
        return -1;
    }

    char buf[MAXREADSIZE];
    //若文件被修改
    if (filelist[fd].fcbstate == 1) {
        do_read(father_fd, filelist[father_fd].length, buf);
        //更新fcb
        fcb *fcbptr = (fcb *) (buf + sizeof(fcb) * filelist[fd].diroff);
        _copy(fd, fcbptr, 1);
        filelist[father_fd].pos = filelist[fd].diroff * sizeof(fcb);
        do_write(father_fd, (char *) fcbptr, sizeof(fcb), 2);
    }

    //释放打开文件表
    memset(&filelist[fd], 0, sizeof(useropen));
    currfd = father_fd;
    return father_fd;
}

int my_write(int fd) {
    if (fd < 0 || fd > MAXOPENFILE) {
        printf("File not exist");
        return -1;
    }
    int wstyle;
    while (1) {
        printf("1: truncate write  2: overwrite  3:append write\n");
        scanf("%d", &wstyle);
        if (wstyle > 3 || wstyle < 1) {
            printf("mode wrong, please input again\n");
        } else {
            break;
        }
    }

    char text[MAXREADSIZE] = "";
    char line[MAXREADSIZE] = "";
    printf("Please enter the file content, if finished please input \":wq\" in new line\n");
    getchar();
    while (gets(line)) {
        if (strcmp(line, ":wq") == 0) {
            break;
        }
        line[strlen(line)] = '\n';
        strcat(text, line);
    }

    text[strlen(text)] = '\0';
    do_write(fd, text, strlen(text) + 1, wstyle);
    filelist[fd].fcbstate = 1;
    return 0;
}

int my_read(int fd){
    if(fd < 0 || fd >= MAXOPENFILE){
        printf("The file not exist");
        return -1;
    }
    filelist[fd].pos = 0;
    char buf[MAXREADSIZE] = "";
    do_read(fd, filelist[fd].length, buf);
    printf("%s\n", buf);
    return 0;
}

int do_write(int fd, char *text, int len, char wstyle){
    if (wstyle == 1){  //清空从头开始写
        filelist[fd].pos = 0;
        filelist[fd].length = 0;
    }else if (wstyle == 3){  //追加写，指针移到末尾
        filelist[fd].pos = filelist[fd].length;
        if(filelist[fd].attribute == ATTR_FILE){
            if(filelist[fd].length!=0){
                filelist[fd].pos = filelist[fd].length -1; //去掉末尾\0
            }
        }
    }

    //找到写指针位置
    int block_num = filelist[fd].first;    //首磁盘块号
    fat *fatptr = (fat*)(myhard + BLOCKSIZE) + block_num;
    int off = filelist[fd].pos;
    while(off >= BLOCKSIZE){
        block_num = fatptr->id;
        if(block_num == END){
            printf("Write pointer fix position failed\n");
            return -1;
        }
        fatptr = (fat*)(myhard + BLOCKSIZE) + block_num;
        off = off - BLOCKSIZE;
    }
    unsigned char *blockptr;
    blockptr = (unsigned char*)(myhard + BLOCKSIZE*block_num);
    char *textptr = text;
    char buf[MAXREADSIZE];
    int len_has_write = 0;
    while(len>len_has_write){
        //把当前磁盘块内容复制到缓冲，一次复制一个磁盘块大小的内容
        memcpy(buf, blockptr, BLOCKSIZE);
        // 从块内偏移开始写，边写边移动偏移量
        for(; off<BLOCKSIZE; off++){
            *(buf + off) = *textptr;
            textptr++;
            len_has_write++;
            if(len == len_has_write)
                break;
        }
        // 把输入写入到磁盘缓冲
        memcpy(blockptr, buf, BLOCKSIZE);

        //写入的内容大于一个磁盘块的话需要写到下一个磁盘块
        //如果没有磁盘块就申请一个
        if(off == BLOCKSIZE && len_has_write != len){
            off = 0;
            block_num = fatptr->id;
            if(block_num == END){
                block_num = get_free_block();
                if(block_num == END){
                    printf("Disk is already full\n");
                    return -1;
                }
                blockptr = (unsigned char*)(myhard + BLOCKSIZE * block_num);
                fatptr->id = block_num;
                fatptr = (fat*)(myhard + BLOCKSIZE) + block_num;
                fatptr->id = END;
            }else{
                blockptr = (unsigned char*)(myhard + BLOCKSIZE * block_num);
                fatptr = (fat*)(myhard + BLOCKSIZE) + block_num;
            }
        }
    }

    //移动读写指针，及原标记+写入长度
    filelist[fd].pos += len;
    //如果写入的过长
    if(filelist[fd].pos > filelist[fd].length){
        filelist[fd].length = filelist[fd].pos;
    }

    //如果1和2模式覆盖的内容小于原文件长度，删除多余的磁盘块
    if(wstyle == 1 || (wstyle == 2 && filelist[fd].attribute == ATTR_DIR)){
        off = filelist[fd].length;
        fatptr = (fat*)(myhard + BLOCKSIZE) + filelist[fd].first;
        while(off >= BLOCKSIZE){
            block_num = fatptr->id;
            off = off-BLOCKSIZE;
            fatptr = (fat*)(myhard + BLOCKSIZE) + block_num;
        }
        while(1){
            //不是最后一块，就先释放这块，再释放后面的
            if(fatptr->id != END){
                int id = fatptr->id;
                fatptr->id = FREE;
                fatptr = (fat*)(myhard + BLOCKSIZE) + id;
            }else{
                fatptr->id = FREE;
                break;
            }
        }

        //FAT表最后一块添加标记
        fatptr = (fat*)(myhard + BLOCKSIZE) + block_num;
        fatptr->id = END;
    }

    //备份fat
    memcpy((fat*)(myhard + BLOCKSIZE*3), (fat*)(myhard + BLOCKSIZE), BLOCKSIZE*2);
    return  len_has_write;
}

int do_read(int fd, int len, char *text) {
    int toread_len = len;
    char *textptr = text;

    int off = filelist[fd].pos;  //块内地址偏移
    int block_num = filelist[fd].first;   //文件起始盘块号
    fat *fat1 = (fat *) (myhard + BLOCKSIZE);
    fat *fatptr = fat1 + block_num;       //FAT表项

    //根据fat表找到目标磁盘块和块内地址
    while (off >= BLOCKSIZE) {
        off = off - BLOCKSIZE;
        block_num = fatptr->id;
        if (block_num == END) {
            printf("load failed\n");
            return -1;
        }
        fatptr = fat1 + block_num;
    }

    //分配缓冲内存
    unsigned char *buf = (unsigned char *) malloc(1024);
    if (buf == NULL) {
        printf("Memory allocation error\n");
        return -1;
    }
    //将整个磁盘块内容读到缓存
    unsigned char *blockptr = myhard + BLOCKSIZE * block_num;
    memcpy(buf, blockptr, BLOCKSIZE);

    while (len > 0) {
        //要读取的内容都在当前磁盘块内
        if (len + off < BLOCKSIZE) {
            memcpy(textptr, buf + off, len);
            textptr += len;
            filelist[fd].pos += len;
            len = 0;
            off += len;
        } else {
            //要读取的内容大于一个磁盘块，先读取当前磁盘块内容
            memcpy(textptr, buf + off, BLOCKSIZE - off);
            textptr += BLOCKSIZE - off;
            len = len - (BLOCKSIZE - off);

            //读下一块
            block_num = fatptr->id;
            if (block_num == END) {
                printf("File attribute length error\n");
                break;
            }
            //移动fat指针和磁盘块指针到下一块
            fatptr = (fat *) (myhard + BLOCKSIZE) + block_num;
            blockptr = myhard + BLOCKSIZE * block_num;
            memcpy(buf, blockptr, BLOCKSIZE);
        }
        free(buf);
        return toread_len - len;
    }
}

//申请空磁盘块,返回空闲磁盘块号
int get_free_block(){
    fat *fat1 = (fat*)(myhard + BLOCKSIZE);
    for(int i=0; i<(int)(SIZE/BLOCKSIZE); i++){
        if(fat1[i].id == FREE){
            return i;
        }
    }
    return -1;
}

//申请空闲文件目录
int get_free_filelist(){
    for(int i=0; i<MAXOPENFILE; i++){
        if(filelist[i].topenfile == 0){
            filelist[i].topenfile = 1;
            return i;
        }
    }
    return -1;
}

void _copy(int fd, fcb *fcbptr, int flag){
    if(flag == 0){
        strcpy(filelist[fd].filename, fcbptr->filename);
        strcpy(filelist[fd].exname, fcbptr->exname);
        filelist[fd].attribute = fcbptr->attribute;
        filelist[fd].date = fcbptr->date;
        filelist[fd].time = fcbptr->time;
        filelist[fd].first = fcbptr->first;
        filelist[fd].free = fcbptr->free;
        filelist[fd].length = fcbptr->length;
    }else{
        strcpy(fcbptr->filename, filelist[fd].filename);
        strcpy(fcbptr->exname, filelist[fd].exname);
        fcbptr->attribute = filelist[fd].attribute;
        fcbptr->date = filelist[fd].date;
        fcbptr->time = filelist[fd].time;
        fcbptr->first = filelist[fd].first;
        fcbptr->free = filelist[fd].free;
        fcbptr->length = filelist[fd].length;
    }
}



