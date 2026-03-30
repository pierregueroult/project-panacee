#include "color.h"

#ifdef USE_MLV
MLV_Color PANACEE_COLOR_ORANGE;
MLV_Color PANACEE_COLOR_GREEN;

void color_init(void) {
	/*#F29400*/
	PANACEE_COLOR_ORANGE = MLV_rgba(242, 148, 0, 255);
	/*#80BA27*/
	PANACEE_COLOR_GREEN = MLV_rgba(128, 186, 39, 255);
}
#endif
