#ifndef __WORLDLOAD_H_
#define __WORLDLOAD_H_

#include <GL/gl.h>

struct chunk {
	struct block {
		int numTris;
		int numVerts;
		struct triangle *tris;
		struct vertex *verts;
		void *_mem;
		GLuint dlist;
	} blocks[16];
};

struct __attribute((packed)) triangle {
	uint8_t vertex[3];
	uint8_t walkability;
	struct __attribute((packed)) uv {
		uint8_t u;
		uint8_t v;
	} uv[3];
	uint16_t texture_and_location;
};

struct __attribute((packed)) vertex {
	int16_t coords[4];
};

extern int makelist();
extern void chunk_inflate(struct chunk *c, void *worldmap, int chunkNum);
extern void chunk_free(struct chunk *chunk);

#endif /* __WORLDLOAD_H_ */
