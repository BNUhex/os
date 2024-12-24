#include <stdio.h>
#include <string.h>
#include "filesys.h"


/*
    dir：显示当前目录下的所有文件和目录
*/

void _dir(){
	unsigned int di_mode;
	int i,j,k;          //xiao
	struct inode *temp_inode;

    // bug04：dir当前目录一直是“..”
    // fix04：创建全局变量，用于保存当前目录的路径名称，并在chdir函数中更新，
    //        dir函数中打印
    printf("\n CURRENT DIRECTORY :%s\n", cur_path_name);
	// printf("\n CURRENT DIRECTORY :%s\n",dir.direct[0].d_name);
	printf("当前共有%d个文件/目录\n",dir.size);
	for (i=0; i<DIRNUM; i++){
		if (dir.direct[i].d_ino != DIEMPTY){

            // 打印文件名
			printf("%-14s", dir.direct[i].d_name);

            // 打印文件权限
			temp_inode = iget(dir.direct[i].d_ino);
			di_mode = temp_inode->di_mode & 00777;
			for (j=0; j<9; j++){
				if (di_mode%2){
					printf("x");
				}else{
					printf("-");
				}
				di_mode = di_mode/2;
			}

			// 打印i节点号
			printf("\ti_ino->%d\t",temp_inode->i_ino);

			// 打印文件类型
			if (temp_inode->di_mode & DIFILE){
				printf(" %ld ", temp_inode->di_size);
				printf("block chain:");
				j=(temp_inode->di_size%BLOCKSIZ)?1:0;
				for (k=0; k < temp_inode->di_size/BLOCKSIZ+j; k++)
					printf("%4d", temp_inode->di_addr[k]);
				printf("\n");
			}else{
				printf("<dir>\n");
			}//else

			// 释放临时内存i节点
			iput(temp_inode);
		}// if (dir.direct[i].d_ino != DIEMPTY)
	}//for
	return;
}

/*
    创建新目录
*/

void mkdir(char *dirname){
	int dirid, dirpos;
	struct inode *inode;
	struct direct buf[BLOCKSIZ/(DIRSIZ+4)];
	unsigned int block;

	dirid= namei(dirname);
	if (dirid != -1){

        //bug03：重复创建目录时提示“dirname是一个文件”而不是提
        //       示“目录dirname已存在”
        //fix03：dirid是当前目录下标，iget(dirid)获得对应的内存目录
        //       项标号，而不是i节点的标号。将dirid改为
        //       dir.direct[dirid].d_ino
		inode = iget(dir.direct[dirid].d_ino);
		if (inode->di_mode & DIDIR)
			printf("目录%s已存在！\n", dirname); //xiao
		else
			printf("%s是一个文件！\n", dirname);
		iput(inode);
		return;
	}
	dirpos = iname(dirname);					//取得在addr中的空闲项位置,并将目录名写到此项里
	inode = ialloc();							//分配i节点
	dir.direct[dirpos].d_ino = inode->i_ino;	//设置该目录的磁盘i节点号
	dir.size++;									//目录数++

	strcpy(buf[0].d_name,"..");					//子目录的上一层目录 当前目录
	buf[0].d_ino = cur_path_inode->i_ino;
	strcpy(buf[1].d_name, ".");
	buf[1].d_ino = inode->i_ino;				//子目录的本目录 子目录

	block = balloc();
	memcpy(disk+DATASTART+block*BLOCKSIZ, buf, BLOCKSIZ);

	inode->di_size = 2*(DIRSIZ+4);
	inode->di_number = 1;
	inode->di_mode = user[user_id].u_default_mode|DIDIR;
	inode->di_uid = user[user_id].u_uid;
	inode->di_gid = user[user_id].u_gid;
	inode->di_addr[0] = block;

	iput(inode);
	return;
}

/*
    切换当前目录至dirname
*/

void chdir(char *dirname){
	unsigned int dirid;
	struct inode *inode;

	unsigned short block;
	int i,j,low=0, high=0;

    // 获取目标目录的索引号
	dirid = namei(dirname);
	if (dirid == -1){
		printf("不存在目录%s！\n", dirname);
		return;
	}

	// 获取目标目录的 i 节点
	inode =iget(dir.direct[dirid].d_ino);
	if(!(inode->di_mode&DIDIR)){
        // bug08：printf没有指出变量%s的内容
        // fix08：添加了dirname
		printf("%s 不是一个目录！\n", dirname);
		return;
	}

    // function03_04：在命令提示符前加入路径
    if (strcmp(dirname, "..") && (strcmp(dirname, "."))) {    // 若不是".."或"."
        strcpy(address[p_address], dirname);
        p_address++;
    } else if ((strcmp(dirname, "..") == 0) && (p_address != 0)) {
        p_address--;
    }

	// 整理当前目录项表，将空闲目录项移到后面
	for (i=0; i<dir.size; i++){
		if(dir.direct[i].d_ino == 0){
			for(j=DIRNUM-1;j>=0&&dir.direct[j].d_ino == 0;j--);
			memcpy(&dir.direct[i], &dir.direct[j], DIRSIZ+4);  //xiao
			dir.direct[j].d_ino = 0;
		}
	}

	// 释放当前目录的所有数据块
	j = cur_path_inode->di_size%BLOCKSIZ?1:0;
	for (i=0; i<cur_path_inode->di_size/BLOCKSIZ+j; i++){
		bfree(cur_path_inode->di_addr[i]);
	}

	// 为当前目录重新分配数据块并写入目录内容
	for (i=0; i<dir.size; i+=BLOCKSIZ/(DIRSIZ+4)){
		block = balloc();
		cur_path_inode->di_addr[i] = block;
		memcpy(disk+DATASTART+block*BLOCKSIZ, &dir.direct[i], BLOCKSIZ);
	}
	// 更新当前目录的 i 节点大小
	cur_path_inode->di_size = dir.size*(DIRSIZ+4);
	iput(cur_path_inode);

	// 将目标目录的 i 节点设置为当前目录
	cur_path_inode = inode;


    // bug04_fix02：更新当前目录路径
    if (strcmp(dirname, "..") == 0) {
        // 返回上一级目录
        char *last_slash = strrchr(cur_path_name, '/');
        if (last_slash && last_slash != cur_path_name) {
            *last_slash = '\0'; // 截断路径
        } else {
            strcpy(cur_path_name, "/"); // 根目录
        }
    } else if (strcmp(dirname, ".") != 0) {
        // 拼接新目录
        if (strcmp(cur_path_name, "/") != 0) {
            strcat(cur_path_name, "/");
        }
        strcat(cur_path_name, dirname);
    }

    // 加载目标目录的数据块内容到内存
	j=0;
	for (i=0; i<inode->di_size/BLOCKSIZ+1; i++){
		memcpy(&dir.direct[j],disk+DATASTART+inode->di_addr[i]*BLOCKSIZ, BLOCKSIZ);
		j+=BLOCKSIZ/(DIRSIZ+4);
	}

	// 更新当前目录大小
	dir.size = cur_path_inode->di_size/(DIRSIZ+4);

	// 将多余的目录项清空
	for (i=dir.size; i<DIRNUM; i++){
		dir.direct[i].d_ino = 0;
	}

	//end by xiao

	return;
}
