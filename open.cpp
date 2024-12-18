#include <stdio.h>
#include "filesys.h"

short open(int user_id, char *filename,char openmode){
	unsigned int dinodeid;
	struct inode *inode;
	int i,j,k;

    // bug06_01：读写不存在的文件时程序崩溃，dinodeid为unsigned int，
    //           不能直接与 -1 进行比较，且这里应该是 dinodeid == -1
    //           而不是 0
    // fix06_01：将 0 改为 (unsigned int)-1
	dinodeid = namei(filename);
	if (dinodeid == (unsigned int)-1){
		printf("\nfile does not existed!!!\n");
		return -1;
	}
	inode = iget(dir.direct[dinodeid].d_ino);
	if (!(inode->di_mode &DIFILE)){
		printf("%s is not a file!!!\n",filename);
		iput(inode);
		return -1;
	}
	if (!access(user_id, inode, openmode)){
		printf("\nfile open has not access!!!\n");
		iput(inode);
		return -1;
	}

	for (i=1; i<SYSOPENFILE; i++){
		if (sys_ofile[i].f_count == 0)
			break;
	}

	if (i == SYSOPENFILE){
		printf("\nsystem open file too much\n");
		iput(inode);
		return -1;
	}

	sys_ofile[i].f_inode = inode;
	sys_ofile[i].f_flag = openmode;
	sys_ofile[i].f_count = 1;

	if (openmode & FAPPEND){
		sys_ofile[i].f_off = inode->di_size;
	}else{
		sys_ofile[i].f_off = 0;
	}

	for (j=0; j<NOFILE; j++){
		if (user[user_id].u_ofile[j] == SYSOPENFILE + 1)
			break;
	}

	if (j == NOFILE){
		printf("\nuser open file too much!!!\n");
		sys_ofile[i].f_count = 0;
		iput(inode);
		return -1;
	}

	user[user_id].u_ofile[j] = i;

	if(openmode & FWRITE){
		k=inode->di_size%BLOCKSIZ?1:0;
		for (i=0; i<inode->di_size/BLOCKSIZ+k; i++)
			bfree(inode->di_addr[i]);
		inode->di_size = 0;
	}
	return j;
}




