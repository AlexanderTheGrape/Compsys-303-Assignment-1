/*
 * main.c
 *
 *  Created on: 7/08/2019
 *      Author: lken274
 */


#include <system.h>
#include <altera_avalon_pio_regs.h>
#include <alt_types.h>
#include <sys/alt_irq.h>
#include <stdio.h>
#include <sys/alt_alarm.h>
#include "sccharts.h"

int count = 0;

int main()
{
	printf("running\n");
	reset();

	while(1)
	{
		count = (count + 1) % 800;
		//fetch button inputs
		int buttonVal = IORD_ALTERA_AVALON_PIO_DATA(KEYS_BASE);

		A = !((buttonVal) & (1 << 0));
		B = !((buttonVal) & (1 << 1));
		//A = 1;
		//B = 1;
		//R = 0;
		R = !((buttonVal) & (1 << 2));

		tick();

		IOWR_ALTERA_AVALON_PIO_DATA(LED_RED_BASE, O*233);
		if(count == 0) printf("O: %d, A: %d, B: %d, R: %d\n", O, A, B, R);
	}

	return 0;
}
