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
<<<<<<< HEAD

#include "stdio.h"
#include "user_vcom.h"
#include <map>

=======
#include "user_vcom.h"
#include "queue.h"
#include "Motor.h"
#include "Parser.h"
>>>>>>> feat: add temporaty working files

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

<<<<<<< HEAD
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
=======
/*sends Gcode struct to the queue*/
static void send_to_queue(Gstruct gstruct_to_send){

	if(xQueueSendToBack(cmdQueue, &gstruct_to_send, portMAX_DELAY) == pdPASS){

	}
	else{
		ITM_write("Cannot Send to the Queue\n\r");
	}
}

// Sends reply code to plotter
void send_reply(const char* cmd_type){

	const char* reply_M10 = "M10 XY 380 310 0.00 0.00 A0 B0 H0 S80 U160 D0\r\n";		// reply code for M10 command
	const char* reply_M11 = "M11 1 1 1 1\r\n";												// reply code for M11 command
	const char* reply_OK = "OK\r\n";													// reply code for the rest of the commands

	if(strcmp(cmd_type, "M10") == 0){
		USB_send((uint8_t *)reply_M10, strlen(reply_M10));
		USB_send((uint8_t *)reply_OK, strlen(reply_OK));
	}
	if(strcmp(cmd_type, "M11") == 0){
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
	vTaskDelay(200);

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
			send_to_queue(gstruct_to_send);
			send_reply(gstruct_to_send.cmd_type);


			ITM_write(gcode_str.c_str());
			ITM_write("\n\r");

			memset(gcode_buff, '\0', 60);
			gcode_str = "";
			full_gcode = false;
>>>>>>> feat: add temporaty working files
		}
	}
}

<<<<<<< HEAD
int main(void) {

	prvSetupHardware();

	/* LED1 toggle thread */
	xTaskCreate(test, "test",
			configMINIMAL_STACK_SIZE*2, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(cdc_task, "CDC",
			300, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
=======

volatile uint32_t RIT_count;
xSemaphoreHandle sbRIT = xSemaphoreCreateBinary();
char axis = 'X';
int pps = 80;

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
		if (axis == 'X') {
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

void penMove(int penPos){
	LPC_SCT0->MATCHREL[1].L = 1000 + penPos*1000/255;
}

void static vTaskMotor(void* pvParamters){
	DigitalIoPin bthStepX(0,24, DigitalIoPin::output, true);
	DigitalIoPin bthStepY(0,27, DigitalIoPin::output, true);
	Gstruct gstruct;
	char gcode_buff[60] = {0};
	Motor motor;
	ITM_write("---------------------------    CALIBRATE X  -------------------------\r\n");
	vTaskDelay(1000);
	penMove(255);
	int maxSteps = 0;
	motor.setDirection('X', ISLEFTD);
	bool limitread = motor.readLimit('x');
	axis = 'X';
	while (!limitread){
		limitread = motor.readLimit('x');
		RIT_start(4,pps);
	}

	motor.setDirection('X', !ISLEFTD);

	limitread = motor.readLimit('X');
	while (!limitread){
		limitread = motor.readLimit('X');
		RIT_start(4,pps);
		maxSteps =  maxSteps + 4;
	}

	motor.setLength('X', maxSteps);
	motor.setPos('X', 0);

	ITM_write("---------------------------    CALIBRATE Y  -------------------------\r\n");
	maxSteps = 0;
	motor.setDirection('Y', ISLEFTD);
	limitread = motor.readLimit('Y');
	axis = 'Y';
	while (!limitread){
		limitread = motor.readLimit('Y');
		RIT_start(4,pps);
	}
	motor.setDirection('Y', !ISLEFTD);
	limitread = motor.readLimit('y');
	while (!limitread){
		limitread = motor.readLimit('y');
		RIT_start(4,pps);
		maxSteps =  maxSteps + 4;
	}

	motor.setLength('Y', maxSteps);
	motor.setPos('Y', 0);

	//Set both length to be smaller
	if (motor.getLength('X') <= motor.getLength('Y')) {
		motor.setLength('Y', motor.getLength('X'));
	} else {
		motor.setLength('X', motor.getLength('Y'));
	}

	//Move to middle
	maxSteps = 0;
	motor.setDirection('X', ISLEFTD);
	motor.setDirection('Y', ISLEFTD);
	axis = 'X';
	RIT_start(motor.getLength('X')/2,pps);
	axis = 'Y';
	RIT_start(motor.getLength('X')/2,pps);

	motor.setPos('X', (motor.getLength('X')/2)); // Set position in scale with mDraw
	motor.setPos('Y', (motor.getLength('Y')/2)); // Set position in scale with mDraw

	int64_t newPositionX = 0;
	int64_t newPositionY = 0;
	char buffer[100] = {'\0'};
	uint8_t penUpValue = 255;
	uint8_t penDownValue = 0;
	while(1) {

		if(xQueueReceive(cmdQueue, (void*) &gstruct, (TickType_t) 10)) {
			//if ((gstruct.cmd_type[0] == 'G') && (gstruct.cmd_type[1] == '1')) {
			if (strcmp(gstruct.cmd_type, "G1") == 0) {
				int moveX = 0;
				int moveY = 0;
				int absX = 0;
				int absY = 0;
				newPositionX = gstruct.x_pos*100*motor.getLength('X')/38000;
				newPositionY = gstruct.y_pos*100*motor.getLength('Y')/31000;
				motor.setDirection('X', (newPositionX - motor.getPos('X'))>=0); // if newPositionX is large then move left
				motor.setDirection('Y', (newPositionY - motor.getPos('Y'))>=0); // if newPositionY is large then move down
				absX = abs(newPositionX - motor.getPos('X'));
				absY = abs(newPositionY - motor.getPos('Y'));
				snprintf(buffer, 100, "absX %d, curPosX: %d, newPosX: %lld \r\n", absX, motor.getPos('X'), newPositionX);
				ITM_write(buffer);
				while (moveX < absX) {
					bthStepX.write(moveX%2==0);
					moveX++;
					vTaskDelay(1);
				}
				motor.setPos('X', newPositionX);
				while (moveY < absY) {
					bthStepY.write(moveY%2==0);
					moveY++;
					vTaskDelay(1);
				}
				motor.setPos('Y', newPositionY);

			} else if(strcmp(gstruct.cmd_type,"M1") == 0){
				penMove(gstruct.pen_pos);
			} else if (strcmp(gstruct.cmd_type,"M2") == 0){

			}


			vTaskDelay(1);//
		}
	}
}



static void SCT_Init(){
	Chip_SCT_Init(LPC_SCTLARGE0);
	Chip_SCT_Init(LPC_SCTLARGE1);

	Chip_SWM_MovablePortPinAssign(SWM_SCT0_OUT0_O, 0,10); //pen
	Chip_SWM_MovablePortPinAssign(SWM_SCT1_OUT0_O, 0,12); // laser

	LPC_SCTLARGE0->CONFIG |= (1 << 17); // two 16-bit timers, auto limit

	LPC_SCTLARGE0->CTRL_L |= (72-1) << 5; // set prescaler, SCTimer/PWM clock = 1 MHz

	LPC_SCTLARGE0->MATCHREL[0].L = 20000-1; // match 0 @ 10/1MHz = 10 usec (100 kHz PWM freq) (1MHz/20000)
	LPC_SCTLARGE0->MATCHREL[1].L = 1999; // match 1 used for duty cycle (in 10 steps)
	LPC_SCTLARGE0->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCTLARGE0->EVENT[0].CTRL = (1 << 12); // match 0 condition only
	LPC_SCTLARGE0->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
	LPC_SCTLARGE0->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only
	LPC_SCTLARGE0->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCTLARGE0->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0
	LPC_SCTLARGE0->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg


	//LPC_SCRLARGE1
	LPC_SCTLARGE1->CONFIG |= (1 << 17); // two 16-bit timers, auto limit
	LPC_SCTLARGE1->CTRL_L |= (72-1) << 5; // set prescaler, SCTimer/PWM clock = 1 MHz
	LPC_SCTLARGE1->MATCHREL[0].L = 1000-1; // match 0 @ 10/1MHz = 10 usec (100 kHz PWM freq) (1MHz/1000)
	LPC_SCTLARGE1->MATCHREL[1].L = 999; // match 1 used for duty cycle (in 10 steps)
	LPC_SCTLARGE1->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCTLARGE1->EVENT[0].CTRL = (1 << 12); // match 0 condition only
	LPC_SCTLARGE1->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
	LPC_SCTLARGE1->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only
	LPC_SCTLARGE1->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCTLARGE1->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0
	LPC_SCTLARGE1->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg
}

int main(void) {

	prvSetupHardware();
	SCT_Init();
	ITM_init();

	cmdQueue = xQueueCreate(10, sizeof(Gstruct));


	xTaskCreate(vTaskReceive, "vTaksReceive",
			configMINIMAL_STACK_SIZE + 350, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	xTaskCreate(vTaskMotor, "vTaskMotor",
			configMINIMAL_STACK_SIZE + 300, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(cdc_task, "CDC",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

>>>>>>> feat: add temporaty working files

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
