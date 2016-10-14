#include <devid.h>
#include <dirent.h>
#include <errno.h>
#include <libintl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>

#include <sys/fs/zfs.h>
#include <sys/vdev_impl.h>

#include "compat.h"
#include <assert.h>

/*
 * Write a label block with a ZBT checksum.
 */

static void
label_write(int fd, uint64_t offset, uint64_t size, void *buf)
{
	zio_block_tail_t *zbt, zbt_orig;
	zio_cksum_t zc;

	zbt = (zio_block_tail_t *)((char *)buf + size) - 1;
	zbt_orig = *zbt;

	ZIO_SET_CHECKSUM(&zbt->zbt_cksum, offset, 0, 0, 0);

	zio_checksum(ZIO_CHECKSUM_LABEL, &zc, buf, size);

	VERIFY(pwrite64(fd, buf, size, offset) == size);

	*zbt = zbt_orig;
}

int
main(int argc, char **argv)
{
	int fd;
	vdev_label_t vl;
	nvlist_t *config;
	uberblock_t *ub = (uberblock_t *)vl.vl_uberblock;
	uint64_t txg;
	char *buf;
	size_t buflen;

	VERIFY(argc == 2);
	VERIFY((fd = open(argv[1], O_RDWR | O_LARGEFILE)) != -1);
	VERIFY(pread64(fd, &vl, sizeof (vdev_label_t), 0) ==
			sizeof (vdev_label_t));
	VERIFY(nvlist_unpack(vl.vl_vdev_phys.vp_nvlist,
				sizeof (vl.vl_vdev_phys.vp_nvlist), &config, 0) == 0);
	VERIFY(nvlist_lookup_uint64(config, ZPOOL_CONFIG_POOL_TXG, &txg) == 0);
	VERIFY(txg == 0);
	VERIFY(ub->ub_txg == 0);
	VERIFY(ub->ub_rootbp.blk_birth != 0);

	txg = ub->ub_rootbp.blk_birth;
	ub->ub_txg = txg;

	VERIFY(nvlist_remove_all(config, ZPOOL_CONFIG_POOL_TXG) == 0);
	VERIFY(nvlist_add_uint64(config, ZPOOL_CONFIG_POOL_TXG, txg) == 0);
	buf = vl.vl_vdev_phys.vp_nvlist;
	buflen = sizeof (vl.vl_vdev_phys.vp_nvlist);
	VERIFY(nvlist_pack(config, &buf, &buflen, NV_ENCODE_XDR, 0) == 0);

	label_write(fd, offsetof(vdev_label_t, vl_uberblock),
			1ULL << UBERBLOCK_SHIFT, ub);

	label_write(fd, offsetof(vdev_label_t, vl_vdev_phys),
			VDEV_PHYS_SIZE, &vl.vl_vdev_phys);

	fsync(fd);

	return (0);
}

