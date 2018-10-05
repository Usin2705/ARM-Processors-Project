/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : PN Technologies Inc.
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
#include "FreeRTOS.h"
#include "Parser.h"
#include "ITM_write.h"
#include "task.h"
#include <mutex>
#include "Fmutex.h"
#include "user_vcom.h"
#include "queue.h"

// Queue for Gcode structs
QueueHandle_t cmdQueue;


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
static void prvSetupHardware(void){

	SystemCoreClockUpdate();
	Board_Init();
	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

/*sends Gcode struct to the queue*/
static void send_to_queue(Gstruct gstruct_to_send){

	if(xQueueSendToBack(cmdQueue, &gstruct_to_send, portMAX_DELAY)){

	}
	else{
		ITM_write("Cannot Send to the Queue\n\r");
	}
}

// Sends reply code to plotter
void send_reply(const char* cmd_type){

	const char* reply_M10 = "M10 XY 380 310 0.00 0.00 A0 B0 H0 S80 U160 D90\r\n";		// reply code for M10 command
	const char* reply_M11 = "M11 1 1 1 1\r\n";												// reply code for M11 command
	const char* reply_OK = "OK\r\n";													// reply code for the rest of the commands

	if(strcmp(cmd_type, "M10")){
		USB_send((uint8_t *)reply_M10, strlen(reply_M10));
		USB_send((uint8_t *)reply_OK, strlen(reply_OK));
	}
	if(strcmp(cmd_type, "M11")){
		USB_send((uint8_t *)reply_M11, strlen(reply_M11));
		USB_send((uint8_t *)reply_OK, strlen(reply_OK));
	}
	else{
		USB_send((uint8_t *)reply_OK, strlen(reply_OK));
	}
}


/*Task receives Gcode commands from mDraw program via USB*/
void static vTaskReceive(void* pvParamters){

	Parser parser;
	Gstruct gstruct_to_send;

	std::string gcode_str = "";
	char gcode_buff[60] = {0};
	uint32_t len = 0;

	bool full_gcode = false;

	vTaskDelay(100);

	while(1){

		len = USB_receive((uint8_t *)gcode_buff, 79);

		if(gcode_buff[len - 1] == '\n'){
			gcode_str += gcode_buff;
			gcode_str[gcode_str.length() - 1] = '\0';
			full_gcode = true;
		}
		else{
			gcode_str += gcode_buff;
		}
		/*the full g-code has been received*/
		if(full_gcode){

			gstruct_to_send = parser.parse_gcode(gcode_str.c_str());
			send_reply(gstruct_to_send.cmd_type);

			ITM_write(gcode_str.c_str());
			ITM_write("\n\r");

			memset(gcode_buff, '\0', 60);
			gcode_str = "";
			full_gcode = false;
		}
		vTaskDelay(500);
	}
}

int main(void) {

	prvSetupHardware();
	ITM_init();

	cmdQueue = xQueueCreate(10, sizeof(Gstruct));

	xTaskCreate(vTaskReceive, "vTaksReceive",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	/* LED2 toggle thread */
	xTaskCreate(cdc_task, "CDC",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
