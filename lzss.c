/*
	LZSS.C -- A Data Compression Program

	Based on sample:
	
	4/6/1989 Haruhiko Okumura
	Use, distribute, and modify this program freely.
	Please send me your improved versions.
		PC-VAN		SCIENCE
		NIFTY-Serve	PAF01022
		CompuServe	74050,1022
 */
#include <stdint.h>
#include "lzss.h"

#define EOF ((int)-1)
#define N		 1024	/* size of ring buffer */
#define F		   18	/* upper limit for match_length */
#define THRESHOLD	2   /* encode string into position and length if match_length is greater than this */

static unsigned char lzss_buf[N+F-1];

typedef struct {
	unsigned char *data;
	unsigned long  size;
	unsigned long  offset;
} STREAM_S;

static void lzss_putc(int data, STREAM_S *stream) {
	if (stream->offset < stream->size) {
		stream->data[stream->offset] = data;
		stream->offset++;
	}
}

static int lzss_getc(STREAM_S *stream) {
	int data;
	if (stream->offset < stream->size) {
		data = stream->data[stream->offset];
		stream->offset++;
	} else {
		data = EOF;
	}
	return data;
}

void lzss_decode(const unsigned char *in, int insize, unsigned char *out, int *outsize) {
	int  i, j, k, r, c;
	unsigned int  flags;
	// init io
	STREAM_S infile, outfile;
	infile.data   = (unsigned char*)in;
	infile.size   = insize;
	infile.offset = 0;
	outfile.data   = out;
	outfile.size   = *outsize;
	outfile.offset = 0;
	// decode
	for (i = 0; i < N - F; i++) lzss_buf[i] = ' ';
	r = N - F;  flags = 0;
	for ( ; ; ) {
		if (((flags >>= 1) & 256) == 0) {
			if ((c = lzss_getc(&infile)) == EOF) break;
			flags = c | 0xff00;		/* uses higher byte cleverly */
		}							/* to count eight */
		if (flags & 1) {
			if ((c = lzss_getc(&infile)) == EOF) break;
			lzss_putc(c, &outfile);  lzss_buf[r++] = c;  r &= (N - 1);
		} else {
			if ((i = lzss_getc(&infile)) == EOF) break;
			if ((j = lzss_getc(&infile)) == EOF) break;
			i |= ((j & 0xf0) << 4);  j = (j & 0x0f) + THRESHOLD;
			for (k = 0; k <= j; k++) {
				c = lzss_buf[(i + k) & (N - 1)];
				lzss_putc(c, &outfile);  lzss_buf[r++] = c;  r &= (N - 1);
			}
		}
	}
	// out size
	*outsize = outfile.offset;
}
