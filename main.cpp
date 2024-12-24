#include <stdio.h>
#include "filesys.h"

struct hinode 	hinode[NHINO];

// function03_01：在命令提示符前加入路径
char* address[200]; // 路径
int p_address;      // 索引指针

struct dir 		dir;
struct file 	sys_ofile[SYSOPENFILE];
struct filsys 	filsys;
struct pwd 		pwd[PWDNUM];
struct user 	user[USERNUM];
struct inode* 	cur_path_inode;
int 			user_id;
char   			disk[(DINODEBLK+FILEBLK+2)*BLOCKSIZ];
char 			str[100];
char            cur_path_name[DIRSIZ]; // bug04_fix01：增加当前目录名


int main(){
	int username;
	char password[16];
	user_id = -1;

    // function03_02：在命令提示符前加入路径，初始化路径和索引指针
    p_address = 0;
    for (int i = 0; i < 100; i++) {
        address[i] = (char*)malloc(sizeof(char) * 100);
    }

	// function01_02：改用文件模拟磁盘。判断当前目录下是否存在‘disk.txt’文件
	FILE* fp = fopen("disk.txt", "rb");
	if(fp == NULL){
        format();
	} else {
	    // 初始化内存模拟的磁盘区域，将其全部置0
        memset(disk, 0x00, ((DINODEBLK + FILEBLK + 2) * BLOCKSIZ));
        // 从磁盘文件中读取数据到内存，大小为磁盘模拟大小
        if (fread(disk, sizeof(char), (DINODEBLK + FILEBLK + 2) * BLOCKSIZ, fp)!=
            (DINODEBLK + FILEBLK + 2) * BLOCKSIZ) {
            fprintf(stderr, "Error: Failed to read disk data.\n");
            exit(1);
        }
        // 从内存模拟磁盘的对应位置读取用户密码信息到内存中的‘pwd’结构体
        memcpy(pwd, disk + DATASTART + BLOCKSIZ * 2, BLOCKSIZ);
	}
	fclose(fp);

    //format();
	install();
	printf("Welcome to mini filesystem!\n");
	while(user_id == -1){
		printf("Login:");
		scanf("%d",&username);
		printf("Password:");
		scanf("%s",password);
		user_id = login(username,password);
	}
	do{
        // function03_03：在命令提示符前加入路径
        if (p_address == 0) {
            printf("/");
        }
        for (int i = 0; i < p_address; i++) {
            printf("/%s", address[i]);
        }
		printf("> ");
		fflush(stdin);
		gets(str);
	}while(shell(user_id,str));

	// bug：id不是2118的用户输入exit会报错
	// fix：logout(2118) 改为 logout(username)
	logout(2118);
	halt();
	return 0;
}
