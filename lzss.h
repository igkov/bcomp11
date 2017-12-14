#ifndef __LZSS_H__
#define __LZSS_H__

void lzss_decode(const unsigned char *in, int insize, unsigned char *out, int *outsize);

#endif
