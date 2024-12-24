#include "filesys.h"
#define CLEN 10     // shell命令列表长度
#define CNUM 10     // shell命令最大字符串长度

/*
    解析命令，根据命令执行操作
*/

//enum ctype
char commands[CNUM][CLEN]={
"exit",
"dir",
"mkdir",
"cd",
"mkfile",
"del",
"write",
"read",
"who"       //function02_01：增加查看当前用户信息的功能
};
int getcid(char *command){
	int i;
	if(command == NULL)
		return -1;
	for(i=0;i<CNUM;i++){
		if(strcmp(command,commands[i]) == 0)
			return i;
	}
	return -1;
}
int shell(int user_id,char *str){
	char seps[] =" \t\n\0";
	char* token,*tstr,*buf;
	unsigned short mode,fd;
	int cid,size;
	token = strtok(str,seps);
	if(token == NULL)
		return 1;
	cid = getcid(token);
	switch(cid){

    // dir
	case 1:
		_dir();
		break;

    // mkdir
	case 2:
		token = strtok(NULL,seps);
		if(token == NULL){
			printf("mkdir命令的正确格式为mkdir dirname，请检查命令!\n");
			break;
		}
		mkdir(token);
		break;

    // cd
	case 3:
		token = strtok(NULL,seps);
		if(token == NULL){
			printf("cd命令的正确格式为cd dirname，请检查命令!\n");
			break;
		}
		chdir(token);
		break;

    // mkfile
	case 4:
		token = strtok(NULL,seps);
		if(token == NULL){
			printf("mkfile 命令的正确格式为mkfile filename [mode]，请检查命令!\n");
			break;
		}
		tstr =token;
		mode = DEFAULTMODE;
		token = strtok(NULL,seps);
		if(token != NULL){
			sscanf(token,"%ho",&mode);
		}
		mode = mode|DIFILE|0700;
		fd = creat(user_id,tstr,mode);
		// bug02.2：修改了creat函数的返回的同时也要注意这里unsigned short型
		//          变量fd在直接和-1比较
		// fix02.2：将-1强制转换成unsigned short再和fd比较
		if(fd == (unsigned short)-1){
			printf("创建文件失败！\n");
			break;
		}
		close(user_id,fd);
		break;

    // del
	case 5:
		token = strtok(NULL,seps);
		if(token == NULL){
			printf("del 命令的正确格式为del filename，请检查命令!\n");
			break;
		}
		delete(token);
		break;

    // write
	case 6:
		mode = WRITE;
		token = strtok(NULL,seps);
		tstr = token;
		token = strtok(NULL,seps);
		if(token == NULL){
			printf("write 命令的正确格式为write filename bytes，请检查命令!\n");
			break;
		}
		if(token[0] == '-'){
			if(token[1] == 'a')
				mode = FAPPEND;
		}else{
			sscanf(token,"%d",&size);
		}
		fd = open(user_id,tstr,char(mode));

		// bug06_02：读写不存在的文件时程序崩溃
		// fix06_02：增加对文件是否打开成功的判断
		if (fd == (unsigned short)-1){
            break;
		}
		buf = (char*)malloc(size);
		size = write(fd,buf,size);
		printf("%d bytes have been writed in file %s.\n",size,tstr);
		free(buf);
		close(user_id,fd);
		break;

    // read
	case 7:
		token = strtok(NULL,seps);
		tstr = token;
		token = strtok(NULL,seps);
		if(token == NULL){
			printf("read 命令的正确格式为read filename bytes，请检查命令!\n");
			break;
		}
		sscanf(token,"%d",&size);
		fd = open(user_id,tstr,READ);

		// bug06_03：读写不存在的文件时程序崩溃
		// fix06_03：增加对文件是否打开成功的判断
		if (fd == (unsigned short)-1){
            break;
		}
		buf = (char*)malloc(size+1);
		size = read(fd,buf,size);
		printf("%d bytes have been read in buf from file %s.\n",size,tstr);
		free(buf);
		close(user_id,fd);
		break;

    // who
    case 8:
        // function02_02：增加查看当前用户信息的功能
        who(user_id);
        break;

    // exit
	case 0:
		return 0;

    // default
	default:
		printf("错误:没有命令%s！\n",token);
		break;
	}
	return 1;
}
