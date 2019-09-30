#include <GL/glut.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mapfile.h"
#include "worldload.h"

#define CAMERA_ACCEL_SLOW 100.0
#define CAMERA_ACCEL_FAST 1000.0
#define CAMERA_SPEED_MAX 1000.0

struct vec3 {
	double x, y, z;
};

double vec3mag(struct vec3 *v)
{
	double a = 0.0;
	a += v->x * v->x;
	a += v->y * v->y;
	a += v->z * v->z;
	a = sqrt(a);
	return a;
}

void vec3normalize(struct vec3 *d, struct vec3 *s)
{
	double mag = vec3mag(s);
	d->x = s->x / mag;
	d->y = s->y / mag;
	d->z = s->z / mag;
}

void vec3scale(struct vec3 *d, struct vec3 *s, double t)
{
	d->x = s->x * t;
	d->y = s->y * t;
	d->z = s->z * t;
}

void vec3add(struct vec3 *d, struct vec3 *s, struct vec3 *t)
{
	d->x = s->x + t->x;
	d->y = s->y + t->y;
	d->z = s->z + t->z;
}

double median3d(double a, double b, double c)
{
	double x[3] = { a, b, c };
	void swap(int d, int e) {
		double temp;
		temp = x[d];
		x[d] = x[e];
		x[e] = temp;
	}

	if (x[0] > x[1])
		swap(0, 1);
	if (x[1] > x[2])
		swap(1, 2);
	if (x[0] > x[1])
		swap(0, 1);
	return x[1];
}

struct {
	bool strafeleft;
	bool straferight;
	bool forward;
	bool backward;
	bool turnleft;
	bool turnright;
	bool fast;
} buttons;

struct {
	float accel;
	struct vec3 speed;
	struct vec3 position;
} camera;

extern int die(char *s);

struct chunk *c;

void updateCamera()
{
	struct vec3 direction = { 0.0, 0.0, 0.0 };
	direction.z = buttons.backward - buttons.forward;
	direction.x = buttons.straferight - buttons.strafeleft;
	vec3scale(&direction, &direction, camera.accel);
	vec3add(&camera.speed, &camera.speed, &direction);
	vec3scale(&camera.speed, &camera.speed, 0.9);
	if (vec3mag(&camera.speed) < 0.9) {
		camera.speed.x = 0.0;
		camera.speed.y = 0.0;
		camera.speed.z = 0.0;
	}

	vec3add(&camera.position, &camera.position, &camera.speed);

}

void gfxInit()
{
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	gluPerspective(
		/* FoV degrees */ 90.0,
		/* Aspect ratio */ 4.0 / 3.0,
		/* Z near */ 1.0,
		/* Z far */ 819200.0
	);
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(
		/* Eye position */ 0.0, 2.0 * 8192.0, 2.0 * 8192.0,
		/* Center position */ 0.0, 0.0, 0.0,
		/* Up is +Y */ 0.0, 1.0, 0.0
	);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(0.0, -10.0);
}

void display()
{
	srand(0);
	updateCamera();
	if (vec3mag(&camera.speed) < 0.9 /*an arbitrary small speed */ )
		return;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(-camera.position.x,
		     -camera.position.y, -camera.position.z);

	inline void displayblock(struct chunk *chunk, int blocknum) {
		struct block *b = &(chunk->blocks[blocknum]);
		glCallList(b->dlist);
	}

	for (int t = 0; t < 9; t++) {
	for (int u = 0; u < 7; u++) {
	for (int row = 0; row < 4; row++) {
	for (int column = 0; column < 4; column++) {
		glPushMatrix();
		glTranslatef(8192.0 * 4 * t, 0, 8192.0 * 4 * u);
		glTranslatef(8192.0 * row, 0, 8192.0 * column);
		displayblock(&c[t + u * 9], row + 4 * column);
		glPopMatrix();
	}}}}
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glutSwapBuffers();

	/* screenshot */
	if (0) {
		static int framenum = 0;
		printf("flame %d\n", ++framenum);
		uint8_t *buf = calloc(800*600, 3);
		glReadPixels(0, 0, 800, 600, GL_RGB, GL_UNSIGNED_BYTE, buf);
		char filename[1024];
		sprintf(filename, "frame%08d.bin", framenum);
		FILE *f = fopen(filename,"w");
		fwrite(buf, 800*600, 3, f);
		fclose(f);
		free(buf);
	}

	glutPostRedisplay();
}

void keydown(unsigned char key, int x, int y)
{
	switch (key) {
	case 0x1b:		/* ESC key */
		exit(0);
		break;
	case 'w':		/* falls through */
	case 'W':
		buttons.forward = true;
		break;
	case 's':		/* falls through */
	case 'S':
		buttons.backward = true;
		break;
	case 'a':		/* falls through */
	case 'A':
		buttons.strafeleft = true;
		break;
	case 'd':		/* falls through */
	case 'D':
		buttons.straferight = true;
		break;
	case 'q':		/* falls through */
	case 'Q':
		buttons.turnleft = true;
		break;
	case 'e':		/* falls through */
	case 'E':
		buttons.turnright = true;
		break;
	default:
		printf("keydown: %c\n", key);
		break;
	}
	glutPostRedisplay();
}

void keyup(unsigned char key, int x, int y)
{
	switch (key) {
	case 'w':		/* falls through */
	case 'W':
		buttons.forward = false;
		break;
	case 's':		/* falls through */
	case 'S':
		buttons.backward = false;
		break;
	case 'a':		/* falls through */
	case 'A':
		buttons.strafeleft = false;
		break;
	case 'd':		/* falls through */
	case 'D':
		buttons.straferight = false;
		break;
	case 'q':		/* falls through */
	case 'Q':
		buttons.turnleft = false;
		break;
	case 'e':		/* falls through */
	case 'E':
		buttons.turnright = false;
		break;
	default:
		printf("keyup:   %c\n", key);
		break;
	}
	glutPostRedisplay();
}

void keyspecialdown(int key, int x, int y)
{
	switch (key) {
	case 112:
		buttons.fast = true;
		break;
	default:
		printf("specdown: %d\n", key);
		break;
	}
	glutPostRedisplay();
}

void keyspecialup(int key, int x, int y)
{
	switch (key) {
	case 112:
		buttons.fast = false;
		break;
	default:
		printf("specup:   %d\n", key);
		break;
	}
	glutPostRedisplay();
}

struct chunk *chunks_init(char *filename)
{
	int numChunks = 69;	/* heh */
	struct chunk *c = calloc(numChunks, sizeof(struct chunk));
	if (c == NULL)
		return NULL;
	
	struct MappedFile_s f = MappedFile_Open(filename, false);
	if (!f.data)
		return NULL;
	uint8_t *worldmap = f.data;

	for (int i = 0; i < numChunks; i++) {
		chunk_inflate(&c[i], worldmap, i);
	}

	MappedFile_Close(f);
	return c;
}

// The data for a world is spread across 6 files with different extensions:
//      <world>.bot     replication of .map data, ignored
//      <world>.bsz     models
//      <world>.ev      bytecode for world vm
//      <world>.map     most map data, including mesh
//      <world>s.txz
//      <world>.txz     textures

int worldview(int argc, char *argv[])
{
	glutInit(&argc, argv);
	if (argc < 3)
		die("no world specified");

	camera.position.x = 8192.0 * 4.0 * 4.0;
	camera.position.y = 0;
	camera.position.z = 8192.0 * 4.0 * 3.0;
	camera.speed.x = 1000.0;
	camera.speed.z = 1000.0;

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("ff7tool");
	glutDisplayFunc(display);
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutKeyboardFunc(keydown);
	glutKeyboardUpFunc(keyup);
	glutSpecialFunc(keyspecialdown);
	glutSpecialUpFunc(keyspecialup);

	buttons.strafeleft = false;
	buttons.straferight = false;
	buttons.forward = false;
	buttons.backward = false;
	buttons.fast = false;
	camera.accel = CAMERA_ACCEL_SLOW;

	gfxInit();
	c = chunks_init(argv[2]);
	glutMainLoop();	// never returns
	// we would free() stuff here but again... glut never returns
	return 0;
}
