/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here
#include "FreeRTOS.h"
#include "task.h"
#include "ITM_write.h"
#include <cstring>
#include "DigitalIoPin.h"

#include <mutex>
#include "Fmutex.h"

#include "stdio.h"
#include "user_vcom.h"
#include <map>


// TODO: insert other definitions and declarations here


/* the following is required if runtime statistics are to be collected */
extern "C" {

void vConfigureTimerForRunTimeStats( void ) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}
}
/* end runtime statictics collection */

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);

	ITM_init();

	// initialize RIT (= enable clocking etc.)
	Chip_RIT_Init(LPC_RITIMER);
	// set the priority level of the interrupt
	// The level must be equal or lower than the maximum priority specified in FreeRTOS config
	// Note that in a Cortex-M3 a higher number indicates lower interrupt priority
	NVIC_SetPriority( RITIMER_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 1);
}

/* send data and toggle thread */
static void test(void *xSemaphore) {
	USB_send((uint8_t *)"USB putty running. Program started \r\n", strlen("USB putty running. Program started \r\n"));
	char myBuffer[60] = {'\0'};
	USB_send((uint8_t *) "M10\r\n", strlen("M10\r\n"));
	int X = 380;
	int Y = 310;
	while (1) {
		sprintf(myBuffer, "M10 XY %d %d 0.00 0.00", X,Y);
		USB_send((uint8_t *) myBuffer, 60);
		USB_send((uint8_t *) "A0 B0 H0 S80 U160 D90 \r\nOK\r\n", strlen("A0 B0 H0 S80 U160 D90 \r\nOK\r\n"));
		X= X+ 20;
		Y= Y+ 20;
		if (X>=450) {
			X= 10;
		}
		if (Y>=450) {
			Y= 10;
		}
		vTaskDelay(10);
	}
}

int main(void) {

	prvSetupHardware();

	/* LED1 toggle thread */
	xTaskCreate(test, "test",
			configMINIMAL_STACK_SIZE*2, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(cdc_task, "CDC",
			300, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
