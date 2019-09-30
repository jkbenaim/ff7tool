#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "endian.h"
#include "ff7tool.h"
#include "lzss.h"


struct field {
	void *dat_raw;
	void *mim_raw;
	void *bsx_raw;
	struct {
		void *sections[7];
	} dat;
};

struct script {
	uint16_t	magic;	// always 0502
	uint8_t		num_entities;
	uint8_t		num_models;
	uint16_t	string_offset;
	uint16_t	akao_offset;
	uint16_t	scale;
	uint16_t	blank[3];
	char		creator[8];
	char		name[8];
};

void *open_file(char *path, char *suffix)
{
	char *new_path=NULL;
	int fd=-1;
	struct stat sb;
	void *ptr1, *ptr2;
	uint32_t size;

	if(path==NULL)
		die("x");
	if(suffix==NULL)
		die("x");
	new_path=malloc(strlen(path)+4);
	if(new_path==NULL)
		die("died in malloc");
	strcpy(new_path, path);
	strcat(new_path, suffix);
	fd = open(new_path, O_RDONLY);
	if (fd == -1)
		die("couldn't open file");
	if (fstat(fd, &sb) == -1)
		die("couldn't stat file");

	ptr1 = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (ptr1 == NULL)
		die("couldn't mmap");

	ptr2 = malloc(16*1024*1024);
	if (ptr2 == NULL)
		die("couldn't malloc");
	size = lzss_decode(ptr1, ptr2);
	ptr2 = realloc(ptr2, size);

	munmap(ptr1, sb.st_size);
	close(fd);
	free(new_path);
	return ptr2;
}

void open_field(struct field *f, char *path)
{
	f->dat_raw = open_file(path, ".dat");
	f->mim_raw = open_file(path, ".mim");
	f->bsx_raw = open_file(path, ".bsx");

	uint32_t *t = f->dat_raw;
	for(int i=0; i<7; i++) {
		f->dat.sections[i] = f->dat_raw + le32toh(t[i]) + 28 - le32toh(t[0]);
	}

}

void close_field(struct field *f)
{
	free(f->dat_raw);
	free(f->mim_raw);
	free(f->bsx_raw);
}

int fieldview (int argc, char *argv[])
{
	struct field f;

	if(argc < 2)
		die("need an argument");

	open_field(&f, argv[1]);

	struct script *s = (struct script *)(f.dat.sections[0]);

	printf("%s %s\n", s->name, s->creator);
	printf("%hu\n", s->num_entities);

	for(int i=0;i<s->num_entities;i++) {
		char *t = (char *)(f.dat.sections[0] + sizeof(struct script) + 8*i);
		printf("%s\n", t);
	}


	close_field(&f);
	return 0;
}

