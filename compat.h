
void
byteswap_uint64_array(void *vbuf, size_t size)
{
	uint64_t *buf = vbuf;
	size_t count = size >> 3;
	int i;

	ASSERT((size & 7) == 0);

	for (i = 0; i < count; i++)
		buf[i] = BSWAP_64(buf[i]);
}

int
zio_handle_fault_injection(zio_t *zio, int error)
{
	return 0;
}

void
zio_set_gang_verifier(zio_t *zio, zio_cksum_t *zcp)
{
	return;
}

uint32_t zio_injection_enabled;

#define VERIFY assert
