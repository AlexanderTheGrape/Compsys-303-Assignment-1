#include <stdio.h>
#include "sys/alt_alarm.h"
#include <altera_avalon_pio_regs.h>
#include <alt_types.h>
#include <system.h>
#include "sys/alt_irq.h"

// ------------- Variables ----------------------------
int mode = 1;

#define RR 		0b100100
#define GR 		0b100001
#define YR 		0b100010
#define RG 		0b001100
#define RY 		0b010100
#define GRped 0b01100001
#define RGped 0b10001100
int currentState = RR;
int prevState = RY;

alt_alarm timer;
int msCount = 0;

FILE *lcd;

unsigned int LEDbits = 0;

unsigned int uiButtonValue = 0;
void * buttonContext = (void*) &uiButtonValue;
int pedNS_pressed = 0;
int pedEW_pressed = 0;

// ------------- Main Code ----------------------------



//alt_irq_register irqReg;

alt_u32 tlc_timer_isr(void* context)
{
	int *timeCount = (int*) context;

	msCount = msCount + 500;

	return 500;
}

void NSEW_ped_isr(void* context)
{
	int *timeCount = (int*) context;

	int uiButtonValue = IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE);
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE, 0);// clear the edge capture register

	int maskedButton0 = uiButtonValue & (1 << 0);
	int maskedButton1 = uiButtonValue & (1 << 1);

	if (maskedButton0 > 0){
			pedNS_pressed = 1;
	} else if (maskedButton1 > 0){
			pedEW_pressed = 1;
	}
}

void lcd_set_mode(int newMode){
	mode = newMode;

	#define ESC 27
	#define CLEAR_LCD_STRING "[2J"
	fprintf(lcd, "%c%s", ESC, CLEAR_LCD_STRING);
	fprintf(lcd, "MODE: %d\n", mode);
}

void init_buttons_pio(){
	unsigned int uiButton = 0;

	// clears the edge capture register
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE, 0);

	// enable interrupts for all buttons
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEYS_BASE, 0x7);

	// start the button isr
	alt_irq_register(KEYS_IRQ, buttonContext, NSEW_ped_isr);
}

void simple_tlc(){
		//Initial state RR, previous state RY

		// Check flags corresponding to seconds elapsed,
		// as well as previous state and current state
		if ((currentState == RR) && (prevState == RY))
		{
			if (msCount >= 500) { // Transition to next state
				if (pedNS_pressed == 1){
					currentState = GRped;
					prevState = RR;
					msCount = 0;
					pedNS_pressed = 0;
				} else { 			// ped button not pressed
					currentState = GR;
					prevState = RR;
					msCount = 0;
				}
			}
		} else if (currentState == GR || currentState == GRped)
		{
			if (msCount >= 6000){
				currentState = YR;
				prevState = GR;
				msCount = 0;
			}
		} else if (currentState == YR)
		{
			if (msCount >= 2000){
				currentState = RR;
				prevState = YR;
				msCount = 0;
			}
		} else if (currentState == RR && prevState == YR)
		{
			if (msCount >= 500){
				if (pedEW_pressed == 1){
					currentState = RGped;
					prevState = RR;
					msCount = 0;
					pedEW_pressed = 0;
				} else {
					currentState = RG;
					prevState = RR;
					msCount = 0;
				}
			}
		} else if (currentState == RG || currentState == RGped)
		{
			if (msCount >= 6000){
				currentState = RY;
				prevState = RG;
				msCount = 0;
			}
		} else if (currentState == RY)
		{
			if (msCount >= 2000){
				currentState = RR;
				prevState = RY;
				msCount = 0;
			}
		}

		// Update output according to new state and previous state
		// Output is on LED's.
		LEDbits = currentState;

		IOWR_ALTERA_AVALON_PIO_DATA(LED_GREEN_BASE, LEDbits);
}

int main()
{
	//Current Mode to be mode 1, displayed on the LCD
	lcd = fopen(LCD_NAME, "w");

	// Start the timer interrupt to start counting to 0.5s
	alt_alarm_start(&timer, 500, tlc_timer_isr, NULL);

	// Set up the button
	init_buttons_pio();

	// Set the mode
	//lcd_set_mode(1);
	lcd_set_mode(2);

	while(1){
		simple_tlc();
	}
	return 0;
}

//
//printf("%d",sizeof(int)); // Show the size of an integer
//
//alt_alarm timer;
//int count;
//
////alt_irq_register irqReg;
//
//alt_u32 timer_isr_function(void* context)
//{
//	int *timeCount = (int*) context;
////	int *uiButtonValue = (int*) buttonContext;
//
//	int uiButtonValue = IORD_ALTERA_AVALON_PIO_DATA(KEYS_BASE);
//
//	int maskedButton1 = uiButtonValue & (1 << 1);
//
//	if ( maskedButton1 == 0){
//		(count)++;
//	}
//	printf("counter value: %d\n", count);
//	return 500;
//}
//
//void button_isr(void* context)
//{
//	int *timeCount = (int*) context;
//
//	//int uiButtonValue = IORD_ALTERA_AVALON_PIO_DATA(KEYS_BASE);
//	int uiButtonValue = IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE);
//	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE, 0);// clear the edge capture register
//
//	int maskedButton0 = uiButtonValue & (1 << 0);
//
//	if (maskedButton0 > 0){
//			(count)++;
//			printf("counter value: %d\n", count);
//	}
//	printf("returning \n");
//}
//
//int main()
//{
//
//
//
//	unsigned int uiButtonValue = 0;
//	void * buttonContext = (void*) &uiButtonValue;
//	count = 0;
//	unsigned int uiSwitchValue = 0;
//	unsigned int uiButton = 0;
//	unsigned int uiButtonValuePrevious = 0;
//	FILE *lcd;
//
//
//	// clears the edge capture register
//	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE, 0);
//
//	// enable interrupts for all buttons
//	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEYS_BASE, 0x7);
//
//	alt_irq_register(KEYS_IRQ, buttonContext, button_isr);
//
//
//
//	//void * actualTimerContext = (void*) &timer;
//	alt_alarm_start(&timer, 500, timer_isr_function, NULL);
//	while(1)
//	{
//		printf("Hello from Nios II!\n");
//
//		IOWR_ALTERA_AVALON_PIO_DATA(LED_GREEN_BASE, 0xaa);
//
//		lcd = fopen(LCD_NAME, "w");
//		while(1)
//		{
//			uiSwitchValue = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);
//			IOWR_ALTERA_AVALON_PIO_DATA(LED_RED_BASE, uiSwitchValue);
//
//			uiButtonValuePrevious = uiButton;
//			//uiButton = IORD_ALTERA_AVALON_PIO_DATA(KEYS_BASE);
//			uiButton = count;
//			if(lcd != NULL)
//			{
//				if(uiButtonValuePrevious != uiButton)
//				{
//					#define ESC 27
//					#define CLEAR_LCD_STRING "[2J"
//					fprintf(lcd, "%c%s", ESC, CLEAR_LCD_STRING);
//					fprintf(lcd, "BUTTON VALUE: %d\n", uiButton);
//				}
//			}
//		}
//
//		fclose(lcd);
//	}
//	return 0;
//}
//
///*
// * "Hello World" example.
// *
// * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
// * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
// * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
// * device in your system's hardware.
// * The memory footprint of this hosted application is ~69 kbytes by default
// * using the standard reference design.
// *
// * For a reduced footprint version of this template, and an explanation of how
// * to reduce the memory footprint for a given application, see the
// * "small_hello_world" template.
// *
// */
//#include <system.h>
//#include <altera_avalon_pio_regs.h>
//#include <stdio.h>
//
//int main()
//{
//	unsigned int uiSwitchValue = 0;
//	unsigned int uiButtonValue = 0;
//	unsigned int uiButtonValuePrevious = 0;
//	FILE *lcd;
//
//	printf("Hello from Nios II!\n");
//
//	IOWR_ALTERA_AVALON_PIO_DATA(LED_GREEN_BASE, 0xaa);
//
//	lcd = fopen(LCD_NAME, "w");
//	while(1)
//	{
//		uiSwitchValue = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);
//		IOWR_ALTERA_AVALON_PIO_DATA(LED_RED_BASE, uiSwitchValue);
//
//		uiButtonValuePrevious = uiButtonValue;
//		uiButtonValue = IORD_ALTERA_AVALON_PIO_DATA(KEYS_BASE);
//		if(lcd != NULL)
//		{
//			if(uiButtonValuePrevious != uiButtonValue)
//			{
//				#define ESC 27
//				#define CLEAR_LCD_STRING "[2J"
//				fprintf(lcd, "%c%s", ESC, CLEAR_LCD_STRING);
//				fprintf(lcd, "BUTTON VALUE: %d\n", uiButtonValue);
//			}
//		}
//	}
//
//	fclose(lcd);
//	return 0;
//}
