#include <stdio.h>
#include <string.h>
#include "filesys.h"

/*
    安装文件系统，将文件系统的各种数据从磁盘(disk变量)读入内存
*/

void install(){
	int i,j;

	/* 1. 从超级块中读取文件系统 */   //xiao
	memcpy(&filsys,disk+BLOCKSIZ,sizeof(struct filsys));

	/* 2. 初始化内存i节点哈希链表 */
	for (i=0; i<NHINO; i++){
		hinode[i].i_forw = NULL;
	}

	/* 3. 初始化系统打开文件表 */
	for (i=0; i<SYSOPENFILE; i++){
		sys_ofile[i].f_count = 0;
		sys_ofile[i].f_inode = NULL;
	}

	/* 4. 初始化用户 */
	for (i=0; i<USERNUM; i++){
		user[i].u_uid = 0;
		user[i].u_gid = 0;
		for (j=0; j<NOFILE; j++)
			user[i].u_ofile[j] = SYSOPENFILE+1;
	}

	/* 5. 读取主目录来初始化dir */
	cur_path_inode = iget(1);
	strcpy(cur_path_name, "/");  // bug04_fix_03：当前目录名
	dir.size = cur_path_inode->di_size/(DIRSIZ+4);//xiao 2-->4

    // bug: 初始化字符串为14个空格，但d_name大小为12，因此可能导致溢出
    // fix: 将字符串初始化为11个空格
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

