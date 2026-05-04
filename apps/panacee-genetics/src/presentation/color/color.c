#include "color.h"

#ifdef USE_MLV
MLV_Color PANACEE_COLOR_ORANGE;
MLV_Color PANACEE_COLOR_GREEN;
MLV_Color PANACEE_COLOR_BLUE;
MLV_Color PANACEE_COLOR_RED;

void color_init(void)
{
    PANACEE_COLOR_ORANGE = MLV_rgba(242, 148,   0, 255); /* #F29400 */
    PANACEE_COLOR_GREEN  = MLV_rgba(128, 186,  39, 255); /* #80BA27 */
    PANACEE_COLOR_BLUE   = MLV_rgba(  0, 150, 199, 255); /* #0096C7 */
    PANACEE_COLOR_RED    = MLV_rgba(214,  40,  40, 255); /* #D62828 */
}
#endif
