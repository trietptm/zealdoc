#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* RC4/ARC4 contains two step:
 * 1. Key-scheduling algorithm(KSA)
 * 2. Pseudo-Random Generation Algorithm(PRGA)
 * */
/* copy from wiki/rc4 */

unsigned char S[256];
unsigned int i, j;
 
/* KSA */
void rc4_init(unsigned char *key, unsigned int key_length)
{
	for (i = 0; i < 256; i++)
		S[i] = i;

	for (i = j = 0; i < 256; i++) {
		unsigned char temp;

		j = (j + key[i % key_length] + S[i]) & 255;
		temp = S[i];
		S[i] = S[j];
		S[j] = temp;
	}

	i = j = 0;
}
 
/* PRGA */
unsigned char rc4_output(void)
{
	unsigned char temp;

	i = (i + 1) & 255;
	j = (j + S[i]) & 255;

	temp = S[j];
	S[j] = S[i];
	S[i] = temp;

	return S[(temp + S[j]) & 255];
}

int main()
{
	int x;
	unsigned char test_vectors[3][2][32] = {
		{"Key", "Plaintext"},
		{"Wiki", "pedia"},
		{"Secret", "Attack at dawn"}
	};

	for (x = 0; x < 3; x++) {
		int y;
		rc4_init(test_vectors[x][0], strlen((char*)test_vectors[x][0]));

		for (y = 0; y < strlen((char*)test_vectors[x][1]); y++)
			printf("%02X", test_vectors[x][1][y] ^ rc4_output());
		printf("\n");
	}
	return 0;
}
