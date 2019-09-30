#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#if defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define be16toh(x) OSSwapBigToHostInt16(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)
#define htobe16(x) OSSwapHostToBigInt16(x)
#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#elif defined(__sgi)
#include <inttypes.h>
uint16_t _bswap16(uint16_t x)
{
	return ((x&0xff)<<8) | ((x&0xff00)>>8);
}
uint32_t _bswap32(uint32_t x)
{
	return (x<<24) | ((x<<8)&0x00ff0000) | ((x>>8)&0x0000ff00) | (x>>24);
}
#define be16toh(x) ((uint16_t)(x))
#define be32toh(x) ((uint32_t)(x))
#define le16toh(x) (_bswap16(x))
#define le32toh(x) (_bswap32(x))
#define htobe16(x) ((uint16_t)(x))
#define htobe32(x) ((uint32_t)(x))
#define htole16(x) (_bswap16(x))
#define htole32(x) (_bswap32(x))
#else
#include <endian.h>
#endif

#endif // _ENDIAN_H_
