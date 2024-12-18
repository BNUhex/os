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

    // 确定需要的盘块数量
    off = sys_ofile[user[user_id].u_ofile[fd]].f_off;
    int remaining_bytes = off + size - inode->di_size;
    blocks_needed = remaining_bytes / BLOCKSIZ;
    if (remaining_bytes % BLOCKSIZ) {
        blocks_needed++;
    }

    // 检查剩余空间是否足够分配
    if (filsys.s_nfree < blocks_needed) {
        printf("Not enough space to write so many bytes!\n");
        return 0;
    }

    // 检查写入是否超出文件限制
    int current_blocks = inode->di_size / BLOCKSIZ;
    if (inode->di_size % BLOCKSIZ) {
        current_blocks++;
    }
    if (current_blocks + blocks_needed > NADDR) {
        printf("Writing so many bytes will exceed the file limit!\n");
        return 0;
    }

    // 为新需要的块分配空间
    for (i = current_blocks; i < current_blocks + blocks_needed; i++) {
        inode->di_addr[i] = balloc();
    }
    inode->di_size += size;

    // 写入数据
    temp_buf = buf;
    off = sys_ofile[user[user_id].u_ofile[fd]].f_off;
    block = off / BLOCKSIZ;
    block_off = off % BLOCKSIZ;

    // 写入当前块的剩余空间
    if (block_off + size < BLOCKSIZ) {
        memcpy(disk + DATASTART + inode->di_addr[block] * BLOCKSIZ + block_off, buf, size);
        sys_ofile[user[user_id].u_ofile[fd]].f_off += size;
        return size;
    }

    // 填充当前块的剩余空间
    memcpy(disk + DATASTART + inode->di_addr[block] * BLOCKSIZ + block_off, temp_buf, BLOCKSIZ - block_off);
    temp_buf += BLOCKSIZ - block_off;
    size -= BLOCKSIZ - block_off;

    // 写入完整的中间块
    for (i = 0; i < size / BLOCKSIZ; i++) {
        memcpy(disk + DATASTART + inode->di_addr[block + 1 + i] * BLOCKSIZ, temp_buf, BLOCKSIZ);
        temp_buf += BLOCKSIZ;
    }

    // 写入最后一块的剩余部分
    int last_block_off = size % BLOCKSIZ;
    if (last_block_off > 0) {
        // bug05：文件写入引起的dir命令错误
        // fix05：将最后一个盘块的地址计算逻辑进行修改
        // memcpy(disk + DATASTART + block * BLOCKSIZ, temp_buf, block_off);
        memcpy(disk + DATASTART + inode->di_addr[block + 1 + i] * BLOCKSIZ, temp_buf, last_block_off);
    }

    // 更新文件偏移
    sys_ofile[user[user_id].u_ofile[fd]].f_off += size + BLOCKSIZ - block_off;
    return size + BLOCKSIZ - block_off;
}


