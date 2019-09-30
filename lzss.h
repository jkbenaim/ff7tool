/* 2015 jrra */

#ifndef _LZSS_H_
#define _LZSS_H_

#include <inttypes.h>

extern uint32_t lzss_get_compressed_size(uint8_t *s);
extern uint32_t lzss_get_decompressed_size(uint8_t *s);
extern uint32_t lzss_decode(uint8_t *s, uint8_t *d);

#endif /* _LZSS_H_ */
