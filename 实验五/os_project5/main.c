#include "fileSys.c"

char *option[12] = {"mkdir", "rmdir", "ls", "cd", "create", "rm", "open", "close", "write", "read", "exit", "help"};

int toNum(char *str) {
    for (int i = 0; i < 12; i++) {
        if (strcmp(str, option[i]) == 0) {
            return i;
        }
    }
    return -1;
}

#define MKDIR 0
#define RMDIR 1
#define LS 2
#define CD 3
#define CREATE 4
#define RM 5
#define OPEN 6
#define CLOSE 7
#define WRITE 8
#define READ 9
#define EXIT 10
#define HELP 11

int main() {
    char command[20];
    int cmd_index = -1;
    initSys();
    while (1) {
        printf("%s> ", filelist[currfd].dir);
        gets(command);
        if (strcmp(command, "") == 0) {
            printf("\n");
            continue;
        }
        char *sp;
        sp = strtok(command, " ");
        cmd_index = toNum(sp);
        switch (cmd_index) {
            case MKDIR:
                sp = strtok(NULL, " ");
                if (sp != NULL) {
                    my_mkdir(sp);
                } else {
                    printf("folder created failed");
                }
                break;
            case CD:
                sp = strtok(NULL, " ");
                if (sp != NULL) {
                    my_cd(sp);
                } else {
                    printf("change wrong");
                }
                break;
            case CREATE:
                sp = strtok(NULL, " ");
                if (sp != NULL) {
                    my_create(sp);
                } else {
                    printf("Create file failed\n");
                }
                break;
            case OPEN:
                sp = strtok(NULL, " ");
                if (sp != NULL) {
                    my_open(sp);
                } else {
                    printf("no %s file\n", sp);
                }
                break;
            case CLOSE:
                sp = strtok(NULL, " ");
                if (filelist[currfd].attribute == 1) {
                    my_close(currfd);
                } else {
                    printf("no file is opened\n", sp);
                }
                break;
            case WRITE:
                if(filelist[currfd].attribute == ATTR_FILE){
                    my_write(currfd);
                } else{
                    printf("Please write in a file");
                }
                break;
            case READ:
                if(filelist[currfd].attribute == ATTR_FILE){
                    my_read(currfd);
                }else{
                    printf("Please open file first");
                }
                break;
            case RM:
                sp = strtok(NULL, " ");
                if (sp != NULL) {
                    my_rm(sp);
                } else {
                    printf("Remove file failed\n");
                }
                break;
            case RMDIR:
                sp = strtok(NULL, " ");
                if (sp != NULL) {
                    my_rmdir(sp);
                } else {
                    printf("Remove folder failed\n");
                }
                break;
            case LS:
                my_ls();
                break;
            case EXIT:
                my_exitsys();
                printf("**************** exit successfully ****************\n");
                return 0;
            default:
                printf("command not found: %s\n", command);
                break;
        }

    }
    my_exitsys();
    return 0;
}