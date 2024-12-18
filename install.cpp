#include <stdio.h>
#include <string.h>
#include "filesys.h"

/*
    ��װ�ļ�ϵͳ�����ļ�ϵͳ�ĸ������ݴӴ���(disk����)�����ڴ�
*/

void install(){
	int i,j;

	/* 1. �ӳ������ж�ȡ�ļ�ϵͳ */   //xiao
	memcpy(&filsys,disk+BLOCKSIZ,sizeof(struct filsys));

	/* 2. ��ʼ���ڴ�i�ڵ��ϣ���� */
	for (i=0; i<NHINO; i++){
		hinode[i].i_forw = NULL;
	}

	/* 3. ��ʼ��ϵͳ���ļ��� */
	for (i=0; i<SYSOPENFILE; i++){
		sys_ofile[i].f_count = 0;
		sys_ofile[i].f_inode = NULL;
	}

	/* 4. ��ʼ���û� */
	for (i=0; i<USERNUM; i++){
		user[i].u_uid = 0;
		user[i].u_gid = 0;
		for (j=0; j<NOFILE; j++)
			user[i].u_ofile[j] = SYSOPENFILE+1;
	}

	/* 5. ��ȡ��Ŀ¼����ʼ��dir */
	cur_path_inode = iget(1);
	strcpy(cur_path_name, "/");  // bug04_fix_03����ǰĿ¼��
	dir.size = cur_path_inode->di_size/(DIRSIZ+4);//xiao 2-->4

    // bug: ��ʼ���ַ���Ϊ14���ո񣬵�d_name��СΪ12����˿��ܵ������
    // fix: ���ַ�����ʼ��Ϊ11���ո�
	for (i=0; i<DIRNUM; i++){
		strcpy(dir.direct[i].d_name,"           ");
		dir.direct[i].d_ino = 0;
	}

	for (i=0; i<dir.size/(BLOCKSIZ/(DIRSIZ+4)); i++){
		memcpy(&dir.direct[(BLOCKSIZ/(DIRSIZ+4))*i],
		disk+DATASTART+BLOCKSIZ*cur_path_inode->di_addr[i], DINODESIZ);
	}

	memcpy(&dir.direct[(BLOCKSIZ)/(DIRSIZ+4)*i],
	disk+DATASTART+BLOCKSIZ*cur_path_inode->di_addr[i], DINODESIZ);
	return;
}

