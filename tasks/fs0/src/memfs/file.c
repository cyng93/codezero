/*
 * Memfs file operations
 *
 * Copyright (C) 2008 Bahadir Balban
 */
#include <fs.h>
#include <vfs.h>
#include <file.h>
#include <memfs/memfs.h>
#include <stdio.h>
#include <string.h>
#include <l4/macros.h>
#include <l4/api/errno.h>
#include INC_GLUE(memory.h)


#if 0

/* 
 * FIXME: read_write() could be more layered using these functions.
 */
void *memfs_read_block(struct vnode *v, int blknum)
{
	void *buf = vfs_alloc_block();

	if (!buf)
		return PTR_ERR(-ENOMEM);

	if(!v->block[blknum])
		return PTR_ERR(-EEXIST);

	memcpy(buf, &v->block[blknum], v->sb->blocksize);
	return buf;
}

int memfs_write_block(struct vnode *v, int blknum, void *buf)
{
	if(!v->block[blknum])
		return -EEXIST;

	memcpy(&v->block[blknum], buf, v->sb->blocksize);
	return 0;
}
#endif

/*
 * Handles both read and writes since most details are common.
 */
int memfs_file_read_write(struct vnode *v, unsigned long pfn,
			  unsigned long npages, void *buf, int wr)
{
	struct memfs_inode *i;
	struct memfs_superblock *memfs_sb;
	u32 blocksize;

	/* Don't support different block and page sizes for now */
	BUG_ON(v->sb->blocksize != PAGE_SIZE);

	/* Buffer must be page aligned */
	BUG_ON(!is_page_aligned(buf));

	/* Low-level fs refs must be valid */
	BUG_ON(!(i = v->inode));
	BUG_ON(!(memfs_sb = v->sb->fs_super));
	blocksize = v->sb->blocksize;

	/* Check filesystem per-file size limit */
	if ((pfn + npages) > memfs_sb->fmaxblocks) {
		printf("%s: fslimit: Trying to %s outside maximum file range: %x-%x\n",
		       __FUNCTION__, (wr) ? "write" : "read", pfn, pfn + npages);
		return -EINVAL;	/* Same error that posix llseek returns */
	}

	/* Read-specific operations */
	if (!wr) {
		/* Check if read is beyond EOF */
		if ((pfn + npages) > __pfn(v->size)) {
			printf("%s: Trying to read beyond end of file: %x-%x\n",
			       __FUNCTION__, pfn, pfn + npages);
			return -EINVAL;	/* Same error that posix llseek returns */
		}

		/* Copy the data from inode blocks into page buffer */
		for (int x = pfn, bufpage = 0; x < pfn + npages; x++, bufpage++)
			memcpy(((void *)buf) + (bufpage * blocksize),
			       &i->block[x], blocksize);
	} else { /* Write-specific operations */
		/* Is the write beyond current file size? */
		if (i->size < ((pfn + npages) * (blocksize))) {
			unsigned long diff = pfn + npages - __pfn(i->size);
			unsigned long holes;

			/*
			 * If write is not consecutively after the currently
			 * last file block, the gap must be filled in by holes.
			 */
			if (pfn > __pfn(i->size))
				holes = pfn - __pfn(i->size);

			/* Allocate new blocks */
			for (int x = 0; x < diff; x++)
				if (!(i->block[__pfn(i->size) + x] = memfs_alloc_block(memfs_sb)))
					return -ENOSPC;

			/* Zero out the holes. FIXME: How do we zero out non-page-aligned bytes?` */
			for (int x = 0; x < holes; x++)
				memset(i->block[__pfn(i->size) + x], 0, blocksize);

			/* Update size and the vnode. FIXME: How do we handle non page-aligned size */
			i->size = (pfn + npages) * blocksize;
			v->sb->ops->read_vnode(v->sb, v);
		}

		/* Copy the data from page buffer into inode blocks */
		for (int x = pfn, bufpage = 0; x < pfn + npages; x++, bufpage++)
			memcpy(i->block[x], ((void *)buf) + (bufpage * blocksize), blocksize);
	}

	return npages * blocksize;
}

int memfs_file_write(struct vnode *v, unsigned long pfn, unsigned long npages, void *buf)
{
	return memfs_file_read_write(v, pfn, npages, buf, 1);
}

int memfs_file_read(struct vnode *v, unsigned long pfn, unsigned long npages, void *buf)
{
	return memfs_file_read_write(v, pfn, npages, buf, 0);
}

struct file_ops memfs_file_operations = {
	.read = memfs_file_read,
	.write = memfs_file_write,
};
