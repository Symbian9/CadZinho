#ifndef _SHAPE2_LIB
#define _SHAPE2_LIB

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <locale.h>
#include "graph.h"
#include "dxf.h"

struct Shape{
	long num;
	char *name;
	unsigned char *cmds;
	unsigned int cmd_size;
	struct Shape *next;
}; 
typedef struct Shape shape;

void shx_font_add(shape *shx_font, long num, char *name, unsigned char *cmds, unsigned int cmd_size);

void shx_font_free(shape *shx_font);

void shx_font_print(shape *shx_font);

shape *shx_font_find(shape *shx_font, long num);

shape *shx_font_open(char *path);

graph_obj *shx_font_parse(shape *shx_font, int pool_idx, char *txt, double *w);

#endif