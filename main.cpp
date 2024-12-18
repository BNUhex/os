#include <stdio.h>
#include "filesys.h"

struct hinode 	hinode[NHINO];

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
    format();
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




