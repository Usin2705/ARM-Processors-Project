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
#include "queue.h"
#include "Motor.h"
#include "Parser.h"

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


// Calibrates stepper motor x-axis
void calibrateMotorX(Motor* mtr, DigitalIoPin* btnStepX){

	int maxSteps = 0;
	bool limitread = mtr->readLimitX0();

	ITM_write("---------------------------    CALIBRATE X  -------------------------\r\n");
	vTaskDelay(2000);
	mtr->setDirX(true);

	while (!limitread){

		limitread = mtr->readLimitX0();
		btnStepX->write(true);
		vTaskDelay(1);
		btnStepX->write(false);
		vTaskDelay(1);
	}
	mtr->setDirX(false);
	limitread = mtr->readLimitX1();

	while (!limitread){

		limitread = mtr->readLimitX1();
		btnStepX->write(maxSteps%2==0);
		maxSteps++;
		vTaskDelay(1);
	}
	mtr->setLimDistX(maxSteps);
	mtr->setPosX(0);
}

// Calibrates stepper motor y-axis
void calibrateMotorY(Motor* mtr, DigitalIoPin* btnStepY){

	int maxSteps = 0;
	bool limitread = mtr->readLimitY0();

	ITM_write("---------------------------    CALIBRATE Y  -------------------------\r\n");
	mtr->setDirY(true);// DWN
	limitread = mtr->readLimitY1();

	while (!limitread){
		limitread = mtr->readLimitY1();
		btnStepY->write(true);
		vTaskDelay(1);
		btnStepY->write(false);
		vTaskDelay(1);
	}
	mtr->setDirY(false); // UP
	limitread = mtr->readLimitY0();

	while (!limitread){

		limitread = mtr->readLimitY0();
		btnStepY->write(maxSteps%2==0);
		maxSteps++;
		vTaskDelay(1);
	}
	mtr->setLimDistY(maxSteps);
	mtr->setPosY(0);
}


// moves the motor to the middle
void moveToMiddle(Motor* mtr, DigitalIoPin* bthStepX, DigitalIoPin* bthStepY){

	int maxSteps = 0;

	mtr->setDirX(true);
	mtr->setDirY(true);

	while (maxSteps < (mtr->getLimDistX() / 2)){

		bthStepX->write(maxSteps %2 == 0);
		maxSteps++;
		vTaskDelay(1);
	}
	maxSteps = 0;

	while (maxSteps < (mtr->getLimDistY() / 2)){

		bthStepY->write(maxSteps %2 == 0);
		maxSteps++;
		vTaskDelay(1);
	}

	mtr->setPosX(mtr->getLimDistX() / 2);		// Set position in scale with mDraw
	mtr->setPosY(mtr->getLimDistY() / 2);		// Set position in scale with mDraw
}


// initialzie the motor
void InitMotor(Motor* mtr, DigitalIoPin* bthStepX, DigitalIoPin* bthStepY){

	// calibrate Motors x-axis
	calibrateMotorX(mtr, bthStepX);

	// calibrate Motors y-axis
	calibrateMotorY(mtr, bthStepY);

	//Set both length to be smaller
	if (mtr->getLimDistX() <= mtr->getLimDistY()){
		mtr->setLimDistY(mtr->getLimDistX());
	}
	else{
		mtr->setLimDistX(mtr->getLimDistY());
	}

	// Move the motor to the middle
	moveToMiddle(mtr, bthStepX, bthStepY);
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
		}
		vTaskDelay(500);
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

	Gstruct gstruct;
	char gcode_buff[60] = {0};
	Motor motor;

	// initalize the motor
	InitMotor(&motor, &bthStepX, &bthStepY);

	int64_t newPositionX = 0;
	int64_t newPositionY = 0;
	char buffer[100] = {'\0'};

	while(1) {

		if(xQueueReceive(cmdQueue, (void*) &gstruct, (TickType_t) 10)) {

			if ((gstruct.cmd_type[0] == 'G') && (gstruct.cmd_type[1] == '1')) {

				int moveX = 0;
				int moveY = 0;
				int absX = 0;
				int absY = 0;

				newPositionX = gstruct.x_pos * 100 * motor.getLimDistX() / 38000;
				newPositionY = gstruct.y_pos * 100 * motor.getLimDistY() / 31000;

				// if newPositionX is large then move left
				motor.setDirX( (newPositionX - motor.getPosX() ) >= 0 );
				// if newPositionY is large then move right
				motor.setDirY( (newPositionY - motor.getPosY() ) >= 0);

				absX = abs(newPositionX - motor.getPosX());
				absY = abs(newPositionY - motor.getPosY());

				snprintf(buffer, 100, "absX %d, curPosX: %d, newPosX: %lld \r\n", absX, motor.getPosX(), newPositionX);

				ITM_write(buffer);

				while (moveX < absX) {
					bthStepX.write(moveX%2==0);
					moveX++;
					vTaskDelay(1);
				}
				motor.setPosX(newPositionX);
				while (moveY < absY) {
					bthStepY.write(moveY%2==0);
					moveY++;
					vTaskDelay(1);
				}
				motor.setPosY(newPositionY);

			} else {

			}
		}
	}
}

int main(void) {

	prvSetupHardware();
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


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
