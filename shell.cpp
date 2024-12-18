#include "filesys.h"
#define CLEN 10
#define CNUM 10

/*
    ���������������ִ�в���
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
"read"
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
			printf("mkdir�������ȷ��ʽΪmkdir dirname����������!\n");
			break;
		}
		mkdir(token);
		break;

    // cd
	case 3:
		token = strtok(NULL,seps);
		if(token == NULL){
			printf("cd�������ȷ��ʽΪcd dirname����������!\n");
			break;
		}
		chdir(token);
		break;

    // mkfile
	case 4:
		token = strtok(NULL,seps);
		if(token == NULL){
			printf("mkfile �������ȷ��ʽΪmkfile filename [mode]����������!\n");
			break;
		}
		tstr =token;
		mode = DEFAULTMODE;
		token = strtok(NULL,seps);
		if(token != NULL){
			sscanf(token,"%o",&mode);
		}
		mode = mode|DIFILE|0700;
		fd = creat(user_id,tstr,mode);
		// bug02.2���޸���creat�����ķ��ص�ͬʱҲҪע������unsigned short��
		//          ����fd��ֱ�Ӻ�-1�Ƚ�
		// fix02.2����-1ǿ��ת����unsigned short�ٺ�fd�Ƚ�
		if(fd == (unsigned short)-1){
			printf("�����ļ�ʧ�ܣ�\n");
			break;
		}
		close(user_id,fd);
		break;

    // del
	case 5:
		token = strtok(NULL,seps);
		if(token == NULL){
			printf("del �������ȷ��ʽΪdel filename����������!\n");
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
			printf("write �������ȷ��ʽΪwrite filename bytes����������!\n");
			break;
		}
		if(token[0] == '-'){
			if(token[1] == 'a')
				mode = FAPPEND;
		}else{
			sscanf(token,"%d",&size);
		}
		fd = open(user_id,tstr,char(mode));

		// bug06_02����д�����ڵ��ļ�ʱ�������
		// fix06_02�����Ӷ��ļ��Ƿ�򿪳ɹ����ж�
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
			printf("read �������ȷ��ʽΪread filename bytes����������!\n");
			break;
		}
		sscanf(token,"%d",&size);
		fd = open(user_id,tstr,READ);

		// bug06_03����д�����ڵ��ļ�ʱ�������
		// fix06_03�����Ӷ��ļ��Ƿ�򿪳ɹ����ж�
		if (fd == (unsigned short)-1){
            break;
		}
		buf = (char*)malloc(size+1);
		size = read(fd,buf,size);
		printf("%d bytes have been read in buf from file %s.\n",size,tstr);
		free(buf);
		close(user_id,fd);
		break;

    // exit
	case 0:
		return 0;

    // default
	default:
		printf("����:û������%s��\n",token);
		break;
	}
	return 1;
}
