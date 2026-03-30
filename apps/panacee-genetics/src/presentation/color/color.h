#ifndef COLOR_H
#define COLOR_H

#include <MLV/MLV_all.h>

typedef struct {
	int red;
	int green;
	int blue;
} Color;

extern MLV_Color PANACEE_COLOR_ORANGE;
extern MLV_Color PANACEE_COLOR_GREEN;

void color_init(void);

#endif
