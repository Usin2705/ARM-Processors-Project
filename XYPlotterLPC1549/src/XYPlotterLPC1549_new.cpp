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

#include <mutex>
#include "Fmutex.h"
#include "user_vcom.h"
#include <string.h>

// TODO: insert other definitions and declarations here

SemaphoreHandle_t count_semph;
SemaphoreHandle_t serial_mutex;

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

}

/* send data and toggle thread */
static void send_task(void *pvParameters) {
	bool LedState = false;
	uint32_t count = 0;

	while (1) {
		char str[32] = {"Hello World\n\r"};
		//int len = sprintf(str, "Counter: %lu runs really fast\r\n", count);
		USB_send((uint8_t *)str, 32);
		Board_LED_Set(0, LedState);
		LedState = (bool) !LedState;
		count++;

		vTaskDelay(configTICK_RATE_HZ / 50);
	}
}


/* LED1 toggle thread */
static void receive_task(void *pvParameters) {
	bool LedState = false;

	while (1) {
		char str[80];
		uint32_t len = USB_receive((uint8_t *)str, 79);
		str[len] = 0; /* make sure we have a zero at the end so that we can print the data */
		ITM_write(str);

		Board_LED_Set(1, LedState);
		LedState = (bool) !LedState;
	}
}

void question_task(void* pvParamters){

	static int MAX_LENGHT = 60;
	uint32_t len = 0;
	char str[80] = {0};
	int questions = 0;
	uint32_t buffer_len = 0;
	std::string buffer;


	while(1){

		// tries to take the serial port lock
		if(xSemaphoreTake(serial_mutex, portMAX_DELAY) == pdTRUE){

			len = USB_receive((uint8_t *)str, 79);
			USB_send((uint8_t*)str, len);

			for(int i = 0; i < len; i++){

				if(str[i] == '?' ){
					questions++;
				}

				if(str[i] == '\n' || str[i] == '\r' || i == MAX_LENGHT ){

					buffer = "\r\n[You]: " + buffer + "\r\n";
					buffer_len = buffer.length();
					USB_send((uint8_t *)buffer.c_str(), buffer_len);
					buffer = "";

					for(int j = 0; j != questions; j++){
						xSemaphoreGive(count_semph);
					}
					questions = 0;
				}
				else{
					buffer= buffer + str[i];
					i++;
				}
			}
			// release the serial mutex lock
			xSemaphoreGive(serial_mutex);
			vTaskDelay(1);
		}

	}
}

void answer_task(void* pvParameters){

	std::string answers[5];
	// initialize random seed
	srand(time(0));

	std::string answer = "";
	uint32_t answer_len = 0;
	std::string think = "[Oracle]: I find your lack of faith disturbing\n\r";
	uint32_t think_len = think.length();

	answers[0] = "Maybe";
	answers[1] = "Did you read the data sheet? The answer is right there!";
	answers[2] = "Why would you ask such thing, you sick bastard!";
	answers[3] = "I have the answer, but I wont tell it.";
	answers[4] = "No!";

	SemaphoreHandle_t count_semph = (SemaphoreHandle_t)pvParameters;

	while (1){

		if(xSemaphoreTake(count_semph, portMAX_DELAY) == pdTRUE){

			USB_send((uint8_t *)think.c_str(), think_len);
			vTaskDelay(3000);

			answer = "[Oracle]: " + answers[rand() % 4 + 0] + "\r\n";
			answer_len = answer.length();
			USB_send((uint8_t *)answer.c_str(), answer_len);

		}

	}
}


/*Task receives Gcode commands from mDraw program via USB*/
void static vTaskReceive(void* pvParamters){


	const char* START = "M10";
	const char* XY_TYPE = "XY";
	const char* OK = "\r\nOK\n\r";

	char gcode_buffer[60] = {0}; 				/*G-code will be stored here*/
	int plot_height_mm = 0;
	int plot_width_mm = 0;
	int x_dir = 0;
	int y_dir = 0;

	uint8_t pen_up = 0;
	uint8_t pen_down = 0;

	char str[80] = {0};

	uint32_t len = 0;

	while(1){

		len = USB_receive((uint8_t *)str, 79);
		vTaskDelay(1000);
		ITM_write(str);
		ITM_write("\r\n");
		USB_send((uint8_t *)OK, strlen(OK));
	}
}


static const int MAX_SEM_AMOUNT = 5;
static const int INIT_SEM_AMOUNT = 0;

int main(void) {

	prvSetupHardware();
	ITM_init();
	count_semph = xSemaphoreCreateCounting(MAX_SEM_AMOUNT, INIT_SEM_AMOUNT);
	serial_mutex = xSemaphoreCreateMutex();

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
