#include <stdio.h>
#include <string.h>
#include "filesys.h"


/*
    dir����ʾ��ǰĿ¼�µ������ļ���Ŀ¼
*/

void _dir(){
	unsigned int di_mode;
	int i,j,k;          //xiao
	struct inode *temp_inode;

    // bug04��dir��ǰĿ¼һֱ�ǡ�..��
    // fix04������ȫ�ֱ��������ڱ��浱ǰĿ¼��·�����ƣ�����chdir�����и��£�
    //        dir�����д�ӡ
    printf("\n CURRENT DIRECTORY :%s\n", cur_path_name);
	// printf("\n CURRENT DIRECTORY :%s\n",dir.direct[0].d_name);
	printf("��ǰ����%d���ļ�/Ŀ¼\n",dir.size);
	for (i=0; i<DIRNUM; i++){
		if (dir.direct[i].d_ino != DIEMPTY){

            // ��ӡ�ļ���
			printf("%-14s", dir.direct[i].d_name);

            // ��ӡ�ļ�Ȩ��
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

			// ��ӡi�ڵ��
			printf("\ti_ino->%d\t",temp_inode->i_ino);

			// ��ӡ�ļ�����
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

			// �ͷ���ʱ�ڴ�i�ڵ�
			iput(temp_inode);
		}// if (dir.direct[i].d_ino != DIEMPTY)
	}//for
	return;
}

/*
    ������Ŀ¼
*/

void mkdir(char *dirname){
	int dirid, dirpos;
	struct inode *inode;
	struct direct buf[BLOCKSIZ/(DIRSIZ+4)];
	unsigned int block;

	dirid= namei(dirname);
	if (dirid != -1){

        //bug03���ظ�����Ŀ¼ʱ��ʾ��dirname��һ���ļ�����������
        //       ʾ��Ŀ¼dirname�Ѵ��ڡ�
        //fix03��dirid�ǵ�ǰĿ¼�±꣬iget(dirid)��ö�Ӧ���ڴ�Ŀ¼
        //       ���ţ�������i�ڵ�ı�š���dirid��Ϊ
        //       dir.direct[dirid].d_ino
		inode = iget(dir.direct[dirid].d_ino);
		if (inode->di_mode & DIDIR)
			printf("Ŀ¼%s�Ѵ��ڣ�\n", dirname); //xiao
		else
			printf("%s��һ���ļ���\n", dirname);
		iput(inode);
		return;
	}
	dirpos = iname(dirname);					//ȡ����addr�еĿ�����λ��,����Ŀ¼��д��������
	inode = ialloc();							//����i�ڵ�
	dir.direct[dirpos].d_ino = inode->i_ino;	//���ø�Ŀ¼�Ĵ���i�ڵ��
	dir.size++;									//Ŀ¼��++

	strcpy(buf[0].d_name,"..");					//��Ŀ¼����һ��Ŀ¼ ��ǰĿ¼
	buf[0].d_ino = cur_path_inode->i_ino;
	strcpy(buf[1].d_name, ".");
	buf[1].d_ino = inode->i_ino;				//��Ŀ¼�ı�Ŀ¼ ��Ŀ¼

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
    �л���ǰĿ¼��dirname
*/

void chdir(char *dirname){
	unsigned int dirid;
	struct inode *inode;

	unsigned short block;
	int i,j,low=0, high=0;

    // ��ȡĿ��Ŀ¼��������
	dirid = namei(dirname);
	if (dirid == -1){
		printf("������Ŀ¼%s��\n", dirname);
		return;
	}

	// ��ȡĿ��Ŀ¼�� i �ڵ�
	inode =iget(dir.direct[dirid].d_ino);
	if(!(inode->di_mode&DIDIR)){
        // bug08��printfû��ָ������%s������
        // fix08�������dirname
		printf("%s ����һ��Ŀ¼��\n", dirname);
		return;
	}

	// ����ǰĿ¼���������Ŀ¼���Ƶ�����
	for (i=0; i<dir.size; i++){
		if(dir.direct[i].d_ino == 0){
			for(j=DIRNUM-1;j>=0&&dir.direct[j].d_ino == 0;j--);
			memcpy(&dir.direct[i], &dir.direct[j], DIRSIZ+4);  //xiao
			dir.direct[j].d_ino = 0;
		}
	}

	// �ͷŵ�ǰĿ¼���������ݿ�
	j = cur_path_inode->di_size%BLOCKSIZ?1:0;
	for (i=0; i<cur_path_inode->di_size/BLOCKSIZ+j; i++){
		bfree(cur_path_inode->di_addr[i]);
	}

	// Ϊ��ǰĿ¼���·������ݿ鲢д��Ŀ¼����
	for (i=0; i<dir.size; i+=BLOCKSIZ/(DIRSIZ+4)){
		block = balloc();
		cur_path_inode->di_addr[i] = block;
		memcpy(disk+DATASTART+block*BLOCKSIZ, &dir.direct[i], BLOCKSIZ);
	}
	// ���µ�ǰĿ¼�� i �ڵ��С
	cur_path_inode->di_size = dir.size*(DIRSIZ+4);
	iput(cur_path_inode);

	// ��Ŀ��Ŀ¼�� i �ڵ�����Ϊ��ǰĿ¼
	cur_path_inode = inode;


    // bug04_fix02�����µ�ǰĿ¼·��
    if (strcmp(dirname, "..") == 0) {
        // ������һ��Ŀ¼
        char *last_slash = strrchr(cur_path_name, '/');
        if (last_slash && last_slash != cur_path_name) {
            *last_slash = '\0'; // �ض�·��
        } else {
            strcpy(cur_path_name, "/"); // ��Ŀ¼
        }
    } else if (strcmp(dirname, ".") != 0) {
        // ƴ����Ŀ¼
        if (strcmp(cur_path_name, "/") != 0) {
            strcat(cur_path_name, "/");
        }
        strcat(cur_path_name, dirname);
    }

    // ����Ŀ��Ŀ¼�����ݿ����ݵ��ڴ�
	j=0;
	for (i=0; i<inode->di_size/BLOCKSIZ+1; i++){
		memcpy(&dir.direct[j],disk+DATASTART+inode->di_addr[i]*BLOCKSIZ, BLOCKSIZ);
		j+=BLOCKSIZ/(DIRSIZ+4);
	}

	// ���µ�ǰĿ¼��С
	dir.size = cur_path_inode->di_size/(DIRSIZ+4);

	// �������Ŀ¼�����
	for (i=dir.size; i<DIRNUM; i++){
		dir.direct[i].d_ino = 0;
	}

	//end by xiao

	return;
}










































