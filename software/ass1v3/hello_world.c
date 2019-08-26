#include <stdio.h>
#include "sys/alt_alarm.h"
#include <altera_avalon_pio_regs.h>
#include <alt_types.h>
#include <system.h>
#include "sys/alt_irq.h"
#include <string.h>

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

unsigned int t1=      	500;
unsigned int t2=   	6000;
unsigned int t3=  	2000;
unsigned int t4=	  500;
unsigned int t5=	6000;
unsigned int t6=	   2000;

unsigned int new_times[6];

unsigned int switchValue = 0;
char currentChar;
char charArray[4];
char charSegment[4];
void * switchContext = (void*) &switchValue;

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

void init_switches_pio(){
	unsigned int uiButton = 0;

	// clears the edge capture register
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(SWITCHES_BASE, 0);

	// enable interrupts for all switches
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(SWITCHES_BASE, 0xF);

	// start the switch isr
	alt_irq_register(SWITCHES_IRQ, buttonContext, NSEW_ped_isr);
}

void simple_tlc(){
		//Initial state RR, previous state RY

		// Check flags corresponding to seconds elapsed,
		// as well as previous state and current state
		if ((currentState == RR) && (prevState == RY))
		{
			if (msCount >= t1) { // Transition to next state
				//printf ("%d", switchValue);
				//if (switchValue == 1){
					FILE* fp;

					fp = fopen(UART_NAME, "r+"); //Open file for reading and writing
					if(fp != NULL)
					{
						// check if all characters are received, in particular, the "[end line]"
						unsigned int i = 0;
						unsigned int t = 0;
						currentChar = 0;

						currentChar = fgetc(fp);
						while(currentChar != 'n'){
							printf("%c", currentChar);

							if (currentChar != ','){
								charArray[i] = currentChar;
							} else {
								// Set a null
								if (i < 4) {
									charArray[i] = 0;
								}
								// but if we have a 4 digit number, then the 5th character is going to be set to null
								new_times[t] = atoi(charArray);
								t = t + 1;
								i = 0;
							}

							i = i + 1;
							currentChar = fgetc(fp);
						}





						//---------------------------UART---------------------------------

						FILE* fp;

						fp = fopen("/dev/uart1", "r+"); //Open file for reading and writing
						if(fp)
						{
							/*while (prompt != 'v')
							{
								prompt = getc(fp); //Get a character from the UART
								if (prompt == 't')
								{
									fwrite (msg,strlen(msg),1,fp);
								}
							}
*/						}
						//------------------------UART END---------------------------------

						// Set a null
						if (i < 4) {
							charArray[i] = 0;
						}
						// but if we have a 4 digit number, then the 5th character is going to be set to null
						new_times[t] = atoi(charArray);

						if (t == 5){
							// Assign the new times
							for (unsigned int j=0; j < 6; j++){
								if (j == 0) {
									t1 = new_times[j];
								} else if (j == 1){
									t2 = new_times[j];
								} else if (j == 2){
									t3 = new_times[j];
								} else if (j == 3){
									t4 = new_times[j];
								} else if (j == 4){
									t5 = new_times[j];
								} else {
									t6 = new_times[j];
								}
							}
							printf("\r\n t1: %d \r\n t2: %d \r\n t3: %d \r\n t4: %d \r\n t5: %d \r\n t6: %d ", t1, t2, t3, t4, t5, t6);
						}
					}
				//}
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
			if (msCount >= t2){
				currentState = YR;
				prevState = GR;
				msCount = 0;
			}
		} else if (currentState == YR)
		{
			if (msCount >= t3){
				currentState = RR;
				prevState = YR;
				msCount = 0;
			}
		} else if (currentState == RR && prevState == YR)
		{
			if (msCount >= t4){
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
			if (msCount >= t5){
				currentState = RY;
				prevState = RG;
				msCount = 0;
			}
		} else if (currentState == RY)
		{
			if (msCount >= t6){
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
//	lcd_set_mode(2);
	lcd_set_mode(3);



//	char* msg = "Detected the character 't'.\r\n";
//	FILE* fp;
//	char prompt;
//
//	fp = fopen(UART_NAME, "r+"); //Open file for reading and writing
//	//usleep(200000);
//	if(fp != NULL)
//	{
//		while (prompt != 'v')
//		{
//			prompt = getc(fp); //Get a character from the UART
//			if (prompt == 't')
//			{
//				fwrite (msg,strlen(msg),1,fp);
//				//printf ("hello"); // use %s when printing
//			}
//		}
//	}

	while(1){
		simple_tlc();
//		prompt = fgetc(fp);
//		printf ("%c", prompt);
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

