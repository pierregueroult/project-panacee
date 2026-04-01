#ifndef COLOR_H
#define COLOR_H

typedef struct {
	int red;
	int green;
	int blue;
} Color;

#ifdef USE_MLV
#include <MLV/MLV_all.h>

extern MLV_Color PANACEE_COLOR_ORANGE;
extern MLV_Color PANACEE_COLOR_GREEN;
extern MLV_Color PANACEE_COLOR_BLUE;

void color_init(void);
#endif

#endif
