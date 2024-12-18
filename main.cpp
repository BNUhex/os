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
char            cur_path_name[DIRSIZ]; // bug04_fix01�����ӵ�ǰĿ¼��


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

	// bug��id����2118���û�����exit�ᱨ��
	// fix��logout(2118) ��Ϊ logout(username)
	logout(2118);
	halt();
	return 0;
}




