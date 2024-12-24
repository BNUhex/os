#include <stdio.h>
#include <string.h>
#include "filesys.h"

// 停止文件系统的运行，清理资源并安全退出
void halt(){
	int i,j;

	/*1. 写回当前目录的内容 */
	chdir("..");            // 切换到上一级目录
	iput(cur_path_inode);   // 将当前目录的 i 节点写回磁盘

	/*2. 释放用户打开的文件表和系统打开文件表，并释放 i 节点*/
	for(i=0; i<USERNUM; i++){
		if(user[i].u_uid != 0){ // 检查用户是否存在（UID不为0）
			for(j=0; j<NOFILE;j++){
				if(user[i].u_ofile[j] != SYSOPENFILE + 1){  // 检查文件是否已关闭
					close(i,j);                 // 关闭用户打开的文件
					user[i].u_ofile[j] = SYSOPENFILE + 1;   // 将文件描述符置为无效
				}//if
			}//for
		}//if
	}//for

	/*3. 将超级块内容写回磁盘 */
	memcpy(disk+BLOCKSIZ, &filsys, sizeof(struct filsys));

    // function01_01：改用文件模拟磁盘
    FILE* fp = fopen("disk.txt", "wb"); // 打开文件
    fwrite(disk, sizeof(char), (DINODEBLK + FILEBLK + 2) * BLOCKSIZ, fp);

	/*4. 关闭文件系统 */
    fclose(fp);

	/*5. say GOOD BYE to all the user*/
	printf("\nGood Bye. See You Next Time. Please turn off the switch\n");
	exit(0);
}
