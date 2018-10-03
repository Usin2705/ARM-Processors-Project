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
#include <mutex>
#include "Fmutex.h"
#include "user_vcom.h"
#include <string.h>
#include "queue.h"
#include "Motor.h"


// structure for holding the G-code
typedef struct {

	char cmd_type[3];			/*command type of the G-code*/

	float x_pos;				/*goto x position value*/
	float y_pos;				/*goto y position value*/

	int pen_pos;				/*pen position(servo value)*/
	int pen_up;					/*pen down value*/
	int pen_dw;					/*pen up value*/

	int x_dir;					/*stepper x-axis direction*/
	int y_dir;					/*stepper y-axis direction*/

	int speed;					/*speed of the stepper motor*/

	int plot_area_h;			/*height of the plotting area*/
	int plot_area_w;			/*width of the plotting area*/

	int abs;					/*coordinates absolute or relative(absolute: abs = 1)*/

}Gcode;

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

	// initialize RIT (= enable clocking etc.)
		Chip_RIT_Init(LPC_RITIMER);
		// set the priority level of the interrupt
		// The level must be equal or lower than the maximum priority specified in FreeRTOS config
		// Note that in a Cortex-M3 a higher number indicates lower interrupt priority
	NVIC_SetPriority( RITIMER_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 1);
}

/*sends Gcode struct to the queue*/
static void send_gcode(Gcode gcode){

	if(xQueueSendToBack(cmdQueue, &gcode, portMAX_DELAY)){

	}
	else{
		ITM_write("Cannot Send to the Queue\n\r");
	}
}

/*function for parsing the G-code*/
static void parse_gcode(const char* buffer){

	Gcode gcode;
	char cmd[3] = {0};

	sscanf(buffer, "%s ", cmd);

	/*M1: set pen position*/
	if(strcmp(cmd, "M1")){

		sscanf(buffer, "M1 %d", &gcode.pen_pos);
	}
	/*M2: save pen up/down position*/
	if(strcmp(cmd, "M2")){

		sscanf(buffer, "M2 U%d D%d ", &gcode.pen_up, &gcode.pen_dw);
	}
	/*M5: save the stepper directions, plot area and plotting speed*/
	if(strcmp(cmd, "M5")){

		sscanf(buffer, "M5 A%d B%d H%d W%d S%d", &gcode.x_dir, &gcode.y_dir, &gcode.plot_area_h, &gcode.plot_area_w, &gcode.speed);
	}
	/*M10: Log opening in mDraw*/
	if(strcmp(cmd, "M10") == 0){
	}
	/*M11: Limit Status Query*/
	if(strcmp(cmd, "M11") == 0){

	}
	/*G1: Go to position*/
	if(strcmp(cmd, "G1") == 0){

		sscanf(buffer, "G1 X%f Y%f A%d", &gcode.x_pos, &gcode.y_pos, &gcode.abs);
	}
	strcpy(gcode.cmd_type, cmd);

	send_gcode(gcode);
}

/*Task receives Gcode commands from mDraw program via USB*/
void static vTaskReceive(void* pvParamters){

	const char* OK = "\r\nOK\n\r";

	std::string gcode_str = "";
	char gcode_buff[60] = {0};
	uint32_t len = 0;

	bool full_gcode = false;

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

			parse_gcode(gcode_str.c_str());

			ITM_write(gcode_str.c_str());
			ITM_write("\n");
			USB_send((uint8_t *)OK, strlen(OK));
			memset(gcode_buff, '\0', 60);
			gcode_str = "";
			full_gcode = false;
		}
		vTaskDelay(10);
	}
}


volatile uint32_t RIT_count;
xSemaphoreHandle sbRIT = xSemaphoreCreateBinary();
char direction;

extern "C" {

void RIT_IRQHandler(void) {
	static DigitalIoPin bthStepX(0,24, DigitalIoPin::output, true);
	static DigitalIoPin bthStepY(0,27, DigitalIoPin::output, true);

	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	// Tell timer that we have processed the interrupt.
	// Timer then removes the IRQ until next match occurs
	Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
	if(RIT_count > 0) {
		RIT_count--;
		if (direction == 'X') {
			bthStepX.write(RIT_count%2==0);
		} else {
			bthStepY.write(RIT_count%2==0);
		}
	}
	else {
		Chip_RIT_Disable(LPC_RITIMER); // disable timer
		// Give semaphore and set context switch flag if a higher priority task was woken up
		xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
	}
	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}

void RIT_start(int count, int us)
{
	uint64_t cmp_value;
	// Determine approximate compare value based on clock rate and passed interval
	cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;
	// disable timer during configuration
	Chip_RIT_Disable(LPC_RITIMER);
	RIT_count = count;
	// enable automatic clear on when compare value==timer value
	// this makes interrupts trigger periodically
	Chip_RIT_EnableCompClear(LPC_RITIMER);
	// reset the counter
	Chip_RIT_SetCounter(LPC_RITIMER, 0);
	Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
	// start counting
	Chip_RIT_Enable(LPC_RITIMER);
	// Enable the interrupt signal in NVIC (the interrupt controller)
	NVIC_EnableIRQ(RITIMER_IRQn);
	// wait for ISR to tell that we're done
	if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
		// Disable the interrupt signal in NVIC (the interrupt controller)
		NVIC_DisableIRQ(RITIMER_IRQn);
	}
	else {
		// unexpected error
	}
}

void static vTaskMotor(void* pvParamters){
	DigitalIoPin bthStepX(0,24, DigitalIoPin::output, true);
	DigitalIoPin bthStepY(0,27, DigitalIoPin::output, true);
	Gcode gcode;
	char gcode_buff[60] = {0};
	Motor motor;
	ITM_write("---------------------------    CALIBRATE X  -------------------------\r\n");
	vTaskDelay(2000);
	int maxSteps = 0;
	motor.setDirection('X', ISLEFTU);
	bool limitread = motor.readLimit('x');

	while (!limitread){
		limitread = motor.readLimit('x');
		bthStepX.write(true);
		vTaskDelay(1);
		bthStepX.write(false);
		vTaskDelay(1);
	}

	motor.setDirection('X', !ISLEFTU);

	limitread = motor.readLimit('X');
	while (!limitread){
		limitread = motor.readLimit('X');
		bthStepX.write(maxSteps%2==0);
		maxSteps++;
		vTaskDelay(1);
	}

	maxSteps++; // the last step in RIT just don't work.

	motor.setLength('X', maxSteps);
	motor.setPos('X', 0);

	ITM_write("---------------------------    CALIBRATE Y  -------------------------\r\n");
	maxSteps = 0;
	motor.setDirection('Y', !ISLEFTU);
	limitread = motor.readLimit('Y');
	while (!limitread){
		limitread = motor.readLimit('Y');
		bthStepY.write(true);
		vTaskDelay(1);
		bthStepY.write(false);
		vTaskDelay(1);
	}

	motor.setDirection('Y', ISLEFTU);

	limitread = motor.readLimit('y');
	while (!limitread){
		limitread = motor.readLimit('y');
		bthStepY.write(maxSteps%2==0);
		maxSteps++;
		vTaskDelay(1);
	}

	maxSteps++; // the last step in RIT just don't work.

	motor.setLength('Y', maxSteps);
	motor.setPos('Y', 0);

	motor.setDirection('X', ISLEFTU);
	direction = 'X';
	RIT_start((int) (50*motor.getLength('X'))/310, 250);
	direction = 'Y';
	motor.setDirection('Y', !ISLEFTU);
	RIT_start((int) 50*motor.getLength('Y')/310, 250);
	direction = 'X';
	RIT_start((int) 50*motor.getLength('X')/310, 250);



	direction = 'Y';
	RIT_start((int) 50*motor.getLength('Y')/310, 250);

	direction = 'X';
	motor.setDirection('X', !ISLEFTU);
	RIT_start((int) 50*motor.getLength('X')/310, 250);

	direction = 'Y';
	motor.setDirection('Y', ISLEFTU);
	RIT_start((int) 50*motor.getLength('Y')/310, 250);

	while(1) {
		if(xQueueReceive(cmdQueue, (void*) &gcode, (TickType_t) 10)) {
			if ((gcode.cmd_type[0] == 'G') && (gcode.cmd_type[1] == '1')) {

			} else {

			}
		}
	}
}

int main(void) {

	prvSetupHardware();
	ITM_init();

	cmdQueue = xQueueCreate(10, sizeof(Gcode));

	/*
	xTaskCreate(vTaskReceive, "vTaksReceive",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	 */

	xTaskCreate(vTaskMotor, "vTaskMotor",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	/*
	xTaskCreate(cdc_task, "CDC",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);*/


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
