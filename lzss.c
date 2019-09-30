/* 2015 jrra */

#include <inttypes.h>
#include "endian.h"
#include "lzss.h"

uint32_t lzss_get_compressed_size(uint8_t *s)
{
	uint32_t size = le32toh(*(uint32_t*)s);
	return size;
}

/* WARNING: this function sometimes gives the wrong result! */
uint32_t lzss_get_decompressed_size(uint8_t *s)
{
	int bitsLeft=0;
	uint8_t bits=0;
	uint32_t size=0;
	uint8_t *sEnd = s + lzss_get_compressed_size( s ) + 4;

	s += 4;

	while(1)
	{
		if( bitsLeft == 0 )
		{
			if( s >= sEnd )
			{
				return size;
			}
			bitsLeft = 8;
			bits = *(s++);
		}

		if( s >= sEnd )
		{
			return size;
		}

		if( bits & 1 )
		{
			/* literal byte */
			s++;
			size++;
		} else {
			/* backreference */
			int b;
			
			s++;
			b = *(s++);
			size += (b & 0xf) + 3;
		}
		bits >>= 1;
		bitsLeft--;
	}
}


uint32_t lzss_decode(uint8_t *s, uint8_t *d)
{
	int bitsLeft=0;
	uint8_t bits=0;
	uint8_t *dOrig = d;
	uint8_t *sEnd = s + lzss_get_compressed_size( s ) + 4;

	s += 4;

	while(1)
	{
		if( bitsLeft == 0 )
		{
			if( s >= sEnd )
			{
				return (int)(d-dOrig);
			}
			bitsLeft = 8;
			bits = *(s++);
		}

		if( s >= sEnd )
		{
			return (int)(d-dOrig);
		}

		if( bits & 1 )
		{
			/* literal byte */
			*(d++) = *(s++);
		} else {
			/* backreference */
			int a, b;
			uint8_t *realOffset, *dEnd;

			a = *(s++);
			b = *(s++);
			realOffset = d - ((d-dOrig + 0xfee - (a|(b & 0xF0)<<4))&0xfff);
			dEnd = d + (b & 0xf) + 3;
			while( realOffset < dOrig )
			{
				*(d++) = 0;
				realOffset++;
			}
			while( d < dEnd )
			{
				*(d++) = *(realOffset++);
			}
		}
		bits >>= 1;
		bitsLeft--;
	}
}

