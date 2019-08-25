#include <system.h>
#include <altera_avalon_pio_regs.h>
#include <alt_types.h>
#include <sys/alt_irq.h>
#include <stdio.h>

#define ESC 27
#define CLEAR_LCD_STRING "[2J"

int counter = 0;
int counter_prev = -1;

int button0state;
int button1state;

void buttonISR(void* context, alt_u32 ID)
{
	short* buttonVal = (short*)context;
	(*buttonVal) = IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE);
	//clear edge captures
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE, 0);

	button0state = (*buttonVal) & (1 << 0);
	button1state = (*buttonVal) & (1 << 1);

	if(button0state == 1) //if the button 0 has been pressed
	{
		printf("here");
		counter_prev = counter;
		counter++;
	}

	if(button1state == 1)
	{
		//start a timer for 500ms
		counter_prev = counter;
		counter--;
	}

}

int main()
{
	short buttonVal = 0;

	void* buttonContext = (void*) &buttonVal;

	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE, 0); //clear edge captures
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEYS_BASE, 3); //enable interrupts for buttons 0 and 1

	printf("hi");
	//register the isr
	alt_irq_register(KEYS_IRQ, buttonContext, buttonISR);

	FILE *lcd;

	IOWR_ALTERA_AVALON_PIO_DATA(LED_GREEN_BASE, 0xaa);
	lcd = fopen(LCD_NAME, "w");

	while(1)
	{
		if(lcd != NULL)
		{
			if(counter != counter_prev)
			{
				fprintf(lcd, "%c%s", ESC, CLEAR_LCD_STRING);
				fprintf(lcd, "COUNTER: %d\n", counter);
			}
		}
	}
	fclose(lcd);
	return 0;
}
