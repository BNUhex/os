#include <stdio.h>
#include "filesys.h"

unsigned int read(int fd, char *buf, unsigned int size){
	unsigned long off;
	int block, block_off, i, j;
	struct inode *inode;
	char *temp_buf;

	inode = sys_ofile[user[user_id].u_ofile[fd]].f_inode;
	if (!(sys_ofile[user[user_id].u_ofile[fd]].f_flag & FREAD)){
		printf("\nthe file is not opened for read\n");
		return 0;
	}
	temp_buf = buf;
	off = sys_ofile[user[user_id].u_ofile[fd]].f_off;
	if ((off+size) > inode->di_size){
		size = inode->di_size - off;
	}
	block_off = off % BLOCKSIZ;
	block = off / BLOCKSIZ;
	if (block_off+size<BLOCKSIZ){
		memcpy(buf, disk+DATASTART+inode->di_addr[block]*BLOCKSIZ+block_off, size);
		return size;
	}
	memcpy(temp_buf, disk+DATASTART+inode->di_addr[block]*BLOCKSIZ+block_off,BLOCKSIZ-block_off);
	temp_buf += BLOCKSIZ - block_off;
	j = (inode->di_size-off-block_off) / BLOCKSIZ;
	for (i=0; i<(size-(BLOCKSIZ-block_off))/BLOCKSIZ; i++){
		memcpy(temp_buf, disk+DATASTART+inode->di_addr[j+i]*BLOCKSIZ, BLOCKSIZ);
		temp_buf += BLOCKSIZ;
	}

	block_off = (size-(BLOCKSIZ-block_off))% BLOCKSIZ;
	memcpy(temp_buf, disk+DATASTART+i*BLOCKSIZ, block_off);
	sys_ofile[user[user_id].u_ofile[fd]].f_off += size;
	return size;
}


unsigned int write(int fd, char *buf, unsigned int size) {
    unsigned long off;
    int block, block_off, i, blocks_needed;
    struct inode *inode;
    char *temp_buf;

    inode = sys_ofile[user[user_id].u_ofile[fd]].f_inode;
    if (!(sys_ofile[user[user_id].u_ofile[fd]].f_flag & FWRITE)) {
        printf("\nThe file is not opened for write\n");
        return 0;
    }

    // ȷ����Ҫ���̿�����
    off = sys_ofile[user[user_id].u_ofile[fd]].f_off;
    int remaining_bytes = off + size - inode->di_size;
    blocks_needed = remaining_bytes / BLOCKSIZ;
    if (remaining_bytes % BLOCKSIZ) {
        blocks_needed++;
    }

    // ���ʣ��ռ��Ƿ��㹻����
    if (filsys.s_nfree < blocks_needed) {
        printf("Not enough space to write so many bytes!\n");
        return 0;
    }

    // ���д���Ƿ񳬳��ļ�����
    int current_blocks = inode->di_size / BLOCKSIZ;
    if (inode->di_size % BLOCKSIZ) {
        current_blocks++;
    }
    if (current_blocks + blocks_needed > NADDR) {
        printf("Writing so many bytes will exceed the file limit!\n");
        return 0;
    }

    // Ϊ����Ҫ�Ŀ����ռ�
    for (i = current_blocks; i < current_blocks + blocks_needed; i++) {
        inode->di_addr[i] = balloc();
    }
    inode->di_size += size;

    // д������
    temp_buf = buf;
    off = sys_ofile[user[user_id].u_ofile[fd]].f_off;
    block = off / BLOCKSIZ;
    block_off = off % BLOCKSIZ;

    // д�뵱ǰ���ʣ��ռ�
    if (block_off + size < BLOCKSIZ) {
        memcpy(disk + DATASTART + inode->di_addr[block] * BLOCKSIZ + block_off, buf, size);
        sys_ofile[user[user_id].u_ofile[fd]].f_off += size;
        return size;
    }

    // ��䵱ǰ���ʣ��ռ�
    memcpy(disk + DATASTART + inode->di_addr[block] * BLOCKSIZ + block_off, temp_buf, BLOCKSIZ - block_off);
    temp_buf += BLOCKSIZ - block_off;
    size -= BLOCKSIZ - block_off;

    // д���������м��
    for (i = 0; i < size / BLOCKSIZ; i++) {
        memcpy(disk + DATASTART + inode->di_addr[block + 1 + i] * BLOCKSIZ, temp_buf, BLOCKSIZ);
        temp_buf += BLOCKSIZ;
    }

    // д�����һ���ʣ�ಿ��
    int last_block_off = size % BLOCKSIZ;
    if (last_block_off > 0) {
        // bug05���ļ�д�������dir�������
        // fix05�������һ���̿�ĵ�ַ�����߼������޸�
        // memcpy(disk + DATASTART + block * BLOCKSIZ, temp_buf, block_off);
        memcpy(disk + DATASTART + inode->di_addr[block + 1 + i] * BLOCKSIZ, temp_buf, last_block_off);
    }

    // �����ļ�ƫ��
    sys_ofile[user[user_id].u_ofile[fd]].f_off += size + BLOCKSIZ - block_off;
    return size + BLOCKSIZ - block_off;
}


