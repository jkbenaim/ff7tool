#include <GL/glut.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "endian.h"
#include "worldload.h"
#include "lzss.h"

extern int die(char *s);

uint32_t wColors[] = {
/*  0 */    0x7fff00,
/*  1 */    0x006400,
/*  2 */    0xdeb887,
/*  3 */    0x00008b,
/*  4 */    0xdc143c,
/*  5 */    0x0000ff,
/*  6 */    0x6495ed,
/*  7 */    0xff7f50,
/*  8 */    0xffebcd,
/*  9 */    0xa9a9a9,
/* 10 */    0xfff8dc,
/* 11 */    0x8a2be2,
/* 12 */    0xbdb76b,
/* 13 */    0xe9967a,
/* 14 */    0x8fbc8f,
/* 15 */    0xff00ff,
/* 16 */    0x1e90ff,
/* 17 */    0xffd700,
/* 18 */    0x008000,
/* 19 */    0xdaa520,
/* 20 */    0x4b0082,
/* 21 */    0xff00ff,
/* 22 */    0x00bfff,
/* 23 */    0xff00ff,
/* 24 */    0xf0e68c,
/* 25 */    0x32cd32,
/* 26 */    0x87cefa,
/* 27 */    0xb0c4de,
/* 28 */    0xffe4b5,
/* 29 */    0x808000,
/* 30 */    0x6b8e23,
/* 31 */    0xff00ff,
};

void randcolor()
{
	float r() {
		return (float)rand()/(float)(1<<31);
	}
	glColor3f(r(), r(), r());
}

void color(int c)
{
	float r,g,b;

	r = ((wColors[c] & 0xFF0000)>>16)/255.0;
	g = ((wColors[c] & 0x00FF00)>> 8)/255.0;
	b = ((wColors[c] & 0x0000FF)    )/255.0;
	glColor3f(r,g,b);
}

void chunk_free(struct chunk *chunk)
{
	int i;
	for(i=0; i<16; i++)
		free(chunk->blocks[i]._mem);
}

void chunk_inflate(struct chunk *c, void *worldmap, int chunkNum)
{
	int i,j,k;
	
	uint8_t *chunkData = (uint8_t *)worldmap + 0xB800 * chunkNum;
	for(i=0;i<16;i++) {
		int32_t blockOffset = le32toh( ((uint32_t *)chunkData)[i] );
		uint8_t *blockData = chunkData + blockOffset;
		uint32_t blockSize = /*lzss_get_decompressed_size(blockData)*/ 65536;
		void *decomp_buffer = malloc(blockSize);
		if( decomp_buffer == NULL )
			die("failure in malloc");
		uint32_t decompressedSize = lzss_decode(blockData, decomp_buffer);
		/*
		if( decompressedSize != blockSize )
			die("discrepancy between expected and real decompressed size");
		*/
		if( decompressedSize <= blockSize )
			realloc(decomp_buffer, decompressedSize);
		else
			die("decomp_buffer too small");

		int numTris = le16toh(((uint16_t *)decomp_buffer)[0]);
		int numVerts = le16toh(((uint16_t *)decomp_buffer)[1]);
		struct triangle *tris = (struct triangle *)(decomp_buffer + 4);
		struct vertex *verts = (struct vertex *)(decomp_buffer + 4 + sizeof(struct triangle)*numTris);
		
		c->blocks[i].numTris = numTris;
		c->blocks[i].numVerts = numVerts;
		c->blocks[i].tris = tris;
		c->blocks[i].verts = verts;
		c->blocks[i]._mem = decomp_buffer;
		
		/* fix endianness of triangles and vertices */
		for(j=0; j<numTris; j++) {
			struct triangle tri = tris[j];
			tri.texture_and_location = le16toh(tri.texture_and_location);
		}
		for(j=0; j<numVerts; j++) {
			for(k=0; k<4; k++) {
				verts[j].coords[k] = le16toh(verts[j].coords[k]);
			}
		}

		/* generate display list for each block */
		struct block *b = &(c->blocks[i]);
		b->dlist = glGenLists(1);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_SHORT, sizeof(struct vertex), b->verts);
		glNewList(b->dlist, GL_COMPILE);
		/* Draw solid tris */
		glColor3f(0.0, 0.0, 0.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_TRIANGLES);
			for(int i=0; i<b->numTris; i++) {
				if(((b->tris[i].walkability)&31)!=3) {
				glArrayElement(b->tris[i].vertex[0]);
				glArrayElement(b->tris[i].vertex[1]);
				glArrayElement(b->tris[i].vertex[2]);
				}
			}
		glEnd();

		/* Draw line tris */
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBegin(GL_TRIANGLES);
			for(int i=0; i<b->numTris; i++) {
				if(((b->tris[i].walkability)&31)!=3) {
				color(b->tris[i].walkability&31);
				glArrayElement(b->tris[i].vertex[0]);
				glArrayElement(b->tris[i].vertex[1]);
				glArrayElement(b->tris[i].vertex[2]);
				}
			}
		glEnd();

		glEndList();

	}
	
}
