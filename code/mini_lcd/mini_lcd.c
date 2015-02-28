#include "mini_lcd_api.h"


void main()
{
	int i = 0;

	init_mini_lcd();

	disp_str(0, "mini lcd test");

	while(1)
	{
		i++
		disp_int(1, i);
		delay_s(1);
		if (i == 100)
			i = 0;
	}
}

