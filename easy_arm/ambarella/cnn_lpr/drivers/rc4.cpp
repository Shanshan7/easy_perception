#include "rc4.h"


RC4::RC4()
{

}

RC4::~RC4()
{

}

void RC4::swap(unsigned char *s, unsigned int i, unsigned int j)
{
	unsigned char temp = s[i];
	s[i] = s[j];
	s[j] = temp;
}

void RC4::rc4_input(unsigned char *pass, unsigned int keylen)
{
	for (i = 0; i < 256; i++)
		s[i] = i;

	for (i = 0, j = 0; i < 256; i++)
	{
		j = (j + pass[i % keylen] + s[i]) & 255;
		swap(s, i, j);
	}
	i = 0;
	j = 0;
}

unsigned char RC4::rc4_output_v01()
{
	i = (i + 1) & 255;
	j = (j + s[i]) & 255;
	swap(s, i, j);

	return s[(s[i] + s[j]) % 255];
}

unsigned char RC4::rc4_output()
{
	i = (i + 1) & 255;
	j = (j + s[i]) & 255;
	swap(s, i, j);

	return s[(s[i] + s[j]) % 256];
}
