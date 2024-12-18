#include <stdio.h>
#include <string.h>
#include "filesys.h"

void format(){
	struct inode *inode;                            // 定义 i 节点指针
	struct direct dir_buf[BLOCKSIZ/(DIRSIZ+4)];     // 目录块缓存
	struct pwd passwd[32];                          // 密码
	unsigned int block_buf[BLOCKSIZ/sizeof(int)];  // 块缓冲区
	int i,j;

	//初始化硬盘
	memset(disk, 0x00, ((DINODEBLK+FILEBLK+2)*BLOCKSIZ));

	/* 0.initialize the passwd */
	passwd[0].p_uid = 2116;
	passwd[0].p_gid = 03;
	strcpy(passwd[0].password, "dddd");

	passwd[1].p_uid = 2117;
	passwd[1].p_gid = 03;
	strcpy(passwd[1].password, "bbbb");

	passwd[2].p_uid = 2118;
	passwd[2].p_gid = 04;
	strcpy(passwd[2].password, "abcd");

	passwd[3].p_uid = 2119;
	passwd[3].p_gid = 04;
	strcpy(passwd[3].password, "cccc");

	passwd[4].p_uid = 2120;
	passwd[4].p_gid = 05;
	strcpy(passwd[4].password, "eeee");

	/* 1.创建主目录、子目录和密码文件 */

    // 1.1 初始化第0个i节点，标记为空
	inode = iget(0);   /* 0 empty dinode id*/
	inode->di_number = 1;			//??空i节点有何用????
	inode->di_mode = DIEMPTY;
	iput(inode);

    // 1.2 创建主目录 i 节点
	inode = iget(1);   /* 1 main dir id*/
	inode->di_number = 1;
	inode->di_mode = DEFAULTMODE | DIDIR;
	inode->di_size = 3*(DIRSIZ + 4);
	inode->di_addr[0] = 0; /*block 0# is used by the main directory*/

    // 初始化主目录内容
	strcpy(dir_buf[0].d_name,"..");
	dir_buf[0].d_ino = 1;
	strcpy(dir_buf[1].d_name,".");
	dir_buf[1].d_ino = 1;
	strcpy(dir_buf[2].d_name,"etc");
	dir_buf[2].d_ino = 2;

    // 将主目录写入磁盘
	memcpy(disk+DATASTART, &dir_buf, 3*(DIRSIZ+4));
	iput(inode);

    // 1.3 创建 etc 子目录
	inode = iget(2);  /* 2  etc dir id */
	inode->di_number = 1;
	inode->di_mode = DEFAULTMODE | DIDIR;
	inode->di_size = 3*(DIRSIZ + 4);
	inode->di_addr[0] = 1; /*block 1# is used by the etc directory*/

    // 初始化etc目录内容
	strcpy(dir_buf[0].d_name,"..");
	dir_buf[0].d_ino = 1;
	strcpy(dir_buf[1].d_name,".");
	dir_buf[1].d_ino = 2;
	strcpy(dir_buf[2].d_name,"password");
	dir_buf[2].d_ino = 3;

    // 将etc目录写入磁盘
	memcpy(disk+DATASTART+BLOCKSIZ*1, dir_buf, 3*(DIRSIZ+4));
	iput(inode);

    // 1.4 创建 password 文件
	inode = iget(3);  /* 3  password id */
	inode->di_number = 1;
	inode->di_mode = DEFAULTMODE | DIFILE;
	inode->di_size = BLOCKSIZ;
	inode->di_addr[0] = 2; /*block 2# is used by the password file*/

    // 初始化密码文件内容
    // bug: password定义的宏大小为12个字符，而这里空格刚好为12个，缺少"\0"的空间导致溢出
    // fix: 将初始化拷贝的字符串长度改为11个空格，留出最后一个字节给"\0"
	for (i=5; i<PWDNUM; i++){
		passwd[i].p_uid = 0;
		passwd[i].p_gid = 0;
		strcpy(passwd[i].password, "            ");  // PWDSIZ " "
	}

    // 将密码数据写入全局变量和磁盘
	memcpy(pwd, passwd, 32*sizeof(struct pwd));
	memcpy(disk+DATASTART+BLOCKSIZ*2, passwd, BLOCKSIZ);
	iput(inode);

	/*2. 初始化超级块 */

	filsys.s_isize = DINODEBLK;                             // i节点块数
	filsys.s_fsize = FILEBLK;                               // 文件块数
	filsys.s_ninode = DINODEBLK * BLOCKSIZ/DINODESIZ - 4;   // 可用i节点数
	filsys.s_nfree = FILEBLK - 3;                           // 可用文件块数

    // 初始化空闲i节点表
	for (i=0; i < NICINOD; i++){
		/* 从第五块开始，前四块已被占用，分别为: 内存i节点表、根目录、etc目录、password文件 */
		filsys.s_inode[i] = 4+i;
	}

	filsys.s_pinode = 0;
	filsys.s_rinode = NICINOD + 4;

    // 初始化空闲数据块链
	block_buf[NICFREE-1] = FILEBLK+1;  /*FILEBLK+1 is a flag of end*/
	for (i=0; i<NICFREE-1; i++)
		block_buf[NICFREE-2-i] = FILEBLK-i-1;			//从最后一个数据块开始分配??????

	memcpy(disk+DATASTART+BLOCKSIZ*(FILEBLK-NICFREE), block_buf, BLOCKSIZ);
	for (i=FILEBLK-2*NICFREE+1; i>2; i-=NICFREE){
		for (j=0; j<NICFREE;j++){
			block_buf[j] = i+j;
		}
		memcpy(disk+DATASTART+BLOCKSIZ*(i-1), block_buf, BLOCKSIZ);
	}
	i += NICFREE;
	j = 1;
	for (; i>3; i--)
	{
		filsys.s_free[NICFREE-j] = i-1;
		j ++;
	}

	filsys.s_pfree = NICFREE - j+1;
	memcpy(disk+BLOCKSIZ, &filsys, sizeof(struct filsys));
	return;

}
