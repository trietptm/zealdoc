
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

static void memset_aligned(void *space, size_t nbytes, unsigned char bits)
{
	assert((nbytes & bits) == 0);
	assert(((uintptr_t)space & bits) == 0);
}
static void memset_16aligned(void *space, size_t nbytes)
{
	memset_aligned(space, nbytes, 0x0F);
}

static void memset_32aligned(void *space, size_t nbytes)
{
	memset_aligned(space, nbytes, 0x1F);
}

static void memset_64aligned(void *space, size_t nbytes)
{
	memset_aligned(space, nbytes, 0x3F);
}

static void memset_128aligned(void *space, size_t nbytes)
{
	memset_aligned(space, nbytes, 0x7F);
}

static void test_mask(size_t align)
{
	uintptr_t mask = ~(uintptr_t)(align - 1);
	void *mem = malloc(1024+align-1);
	void *ptr = (void *)(((uintptr_t)mem+align-1) & mask);

	assert((align & (align - 1)) == 0);

	printf("align: %d\t mask: %0X\n", align, mask);
	printf("0x%08" PRIXPTR ", 0x%08" PRIXPTR "\n", mem, ptr);
	memset_16aligned(ptr, 1024);
	if (align > 16)
		memset_32aligned(ptr, 1024);
	if (align > 32)
		memset_64aligned(ptr, 1024);
	if (align > 64)
		memset_128aligned(ptr, 1024);
	free(mem);
}

int main(void)
{
	test_mask(16);
	test_mask(32);
	test_mask(64);
	test_mask(128);
	return(0);
}

