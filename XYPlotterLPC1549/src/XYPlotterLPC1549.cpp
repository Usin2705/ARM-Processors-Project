/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
 */
#include <bresenham.h>
#include <cr_section_macros.h>

// TODO: insert other include files here
#include "user_vcom.h"
#include "Parser.h"

//#define SIMULATOR
//#define PLOTTER1
//#define PLOTTER2
#define PLOTTER3
//#define LASER

// Queue for gcode command structs
QueueHandle_t cmdQueue;

int PenUP = 0;
int PenDOWN = 0;

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
	NVIC_SetPriority( RITIMER_IRQn, 5);
}

/*sends Gcode struct to the queue*/
static void send_to_queue(Gstruct gstruct_to_send){

	if(xQueueSendToBack(cmdQueue, &gstruct_to_send, portMAX_DELAY) == pdPASS){

	}
	else{
		ITM_write("Cannot Send to the Queue\r\n");
	}
}

// Sends reply code to plotter
void send_reply(Gstruct* g ){


	const char* replyM11 = "M11 1 1 1 1\r\n";												// reply code for M11 command
	char replyM10[100] = {0};																// reply code for M10 command
	const char* replyOK = "OK\r\n";

	sprintf(replyM10, "M10 XY 310 340 0.00 0.00 A0 B0 S80 U%d D%d", g->pen_up, g->pen_dw);


	/*
	#if defined(PLOTTER1)
	reply_M10 = "M10 XY 310 345 0.00 0.00 A0 B0 H0 S80 U0 D180\r\n";		// reply code for M10 command
	#elif defined(PLOTTER2)
	reply_M10 =	"M10 XY 310 340 0.00 0.00 A0 B0 H0 S80 U0 D180\r\n";		// reply code for M10 command
	#elif defined(PLOTTER3)
	reply_M10 =	"M10 XY 310 340 0.00 0.00 A0 B0 H0 S80 U80 D0\r\n";		// reply code for M10 command
	#else
	reply_M10 =	"M10 XY 300 400 0.00 0.00 A0 B0 H0 S80 U160 D90\r\n";
	#endif
	 */

	if(strcmp(g->cmd_type, "M10") == 0){
		USB_send((uint8_t *)replyM10, strlen(replyM10));
		USB_send((uint8_t *)replyOK, strlen(replyOK));
	}
	if(strcmp(g->cmd_type, "M11") == 0){
		USB_send((uint8_t *)replyM11, strlen(replyM11));
		USB_send((uint8_t *)replyOK, strlen(replyOK));
	}
	else{
		USB_send((uint8_t *)replyOK, strlen(replyOK));
	}
}

extern "C"{

// interrupt handler for interrupt pin 0 (swY0)
void PIN_INT0_IRQHandler(void){

	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;

	// clears falling edge detection
	LPC_GPIO_PIN_INT->IST &= ~(1UL << 1);

	// loops infinitely
	while(1);

	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}

// interrupt handler for interrupt pin 1 (swY1)
void PIN_INT1_IRQHandler(void){

	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;

	// clears falling edge detection
	LPC_GPIO_PIN_INT->IST &= ~(1UL << 2);

	// loops infinitely
	while(1);

	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}

// interrupt handler for interrupt pin 2 (swX0)
void PIN_INT2_IRQHandler(void){

	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;

	// clears falling edge detection
	LPC_GPIO_PIN_INT->IST &= ~(1UL << 3);

	// loops infinitely
	while(1);

	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}

// interrupt handler for interrupt pin 3 (swX1)
void PIN_INT3_IRQHandler(void){

	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;

	// clears falling edge detection
	LPC_GPIO_PIN_INT->IST &= ~(1UL << 4);

	// loops infinitely
	while(1);

	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}

}

/* function for initializing limit switches as interrupt pins */
static void interrupt_pins_init(){

	Chip_PININT_Init(LPC_GPIO_PIN_INT);

	DigitalIoPin swY0(0,0, DigitalIoPin::pullup, true);
	DigitalIoPin swY1 (1,3, DigitalIoPin::pullup, true);
	DigitalIoPin swX0 (0,9, DigitalIoPin::pullup, true);
	DigitalIoPin swX1 (0,29, DigitalIoPin::pullup, true);

	/*configuring pins as interrupt pins*/

	Chip_INMUX_PinIntSel(0, 0, 0);			// select pin 0_0(swY0) as interrupt pin 0
	Chip_INMUX_PinIntSel(1, 1, 3);			// select pin 1_3(swY1) as interrupt pin 1
	Chip_INMUX_PinIntSel(2, 0, 9);			// select pin 0_9(swX0) as interrupt pin 2
	Chip_INMUX_PinIntSel(3, 0, 29); 		// select pin 0_29(swX1) as interrupt pin 3

	/*configure selected pins as falling edge interrupts*/
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH0);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH1);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH2);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH3);

	/*configure selected pins as edge sensetive*/
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH0);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH1);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH2);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH3);

	/*Sets priorities for pin interrupts*/
	NVIC_SetPriority(PIN_INT0_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 1);
	NVIC_SetPriority(PIN_INT1_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 1);
	NVIC_SetPriority(PIN_INT2_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 1);
	NVIC_SetPriority(PIN_INT3_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 1);

	/*Enables interrupt signals for the pin interrupts*/
	NVIC_EnableIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);
	NVIC_EnableIRQ(PIN_INT2_IRQn);
	NVIC_EnableIRQ(PIN_INT3_IRQn);
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

			gstruct_to_send = parser.parse_gcode(gcode_str.c_str(), PenUP, PenDOWN);
			send_to_queue(gstruct_to_send);
			send_reply(&gstruct_to_send);

			ITM_write(gcode_str.c_str());
			ITM_write("\r\n");

			memset(gcode_buff, '\0', 60);
			gcode_str = "";
			full_gcode = false;
		}
	}
}



void static vTaskMotor(void* pvParamters){
	Gstruct gstruct;
	Motor motor;

	setLaserPower(0);	//Turn laser off
	motor.setPPS(8000);

#if defined(PLOTTER1)
	penMove(0); 		//Move pen up
#elif defined(PLOTTER2)
	penMove(0); 		//Move pen up
#elif defined(PLOTTER3)
	penMove(80); 		//Move pen up
#else
	penMove(160);		//Move pen up
	motor.setPPS(2500);
#endif

#ifdef LASER
	motor.setPPS(2000);	//Laser drawing should be slower
#endif



	motor.calibrate();
	//moveSquare(&motor);
	//moveRhombus(&motor);
	//moveTrapezoid(&motor);
	interrupt_pins_init();
	char buffer[80] = {'\0'};
	double stepsPerMMX;
	double stepsPerMMY;
	double MMPerStepX;
	double MMPerStepY;

	//lower resolution might be a good idea???

#if defined(PLOTTER1)
	stepsPerMMX = (double) motor.getLimDist(XAXIS)/31000.0; //31cm
	stepsPerMMY = (double) motor.getLimDist(YAXIS)/34500.0; //34.5cm

	MMPerStepX = (double) 31000.0/motor.getLimDist(XAXIS); //31cm
	MMPerStepY = (double) 34500.0/motor.getLimDist(YAXIS); //34.5cm

#elif defined(PLOTTER2) || defined(PLOTTER3)
	stepsPerMMX = (double) motor.getLimDist(XAXIS)/31000.0; //31cm
	stepsPerMMY = (double) motor.getLimDist(YAXIS)/34000.0; //34cm

	MMPerStepX = (double) 31000.0/motor.getLimDist(XAXIS); //31cm
	MMPerStepY = (double) 34000.0/motor.getLimDist(YAXIS); //34cm
#else
	stepsPerMMX = (double) motor.getLimDist(XAXIS)/50000.0;
	stepsPerMMY = (double) motor.getLimDist(YAXIS)/50000.0;

	MMPerStepX = (double) 50000.0/motor.getLimDist(XAXIS);
	MMPerStepY = (double) 50000.0/motor.getLimDist(YAXIS);
#endif

	motor.setScale(XAXIS, stepsPerMMX, MMPerStepX);
	motor.setScale(YAXIS, stepsPerMMY, MMPerStepY);

	while(1) {

		if(xQueueReceive(cmdQueue, (void*) &gstruct, portMAX_DELAY)) {

			if (strcmp(gstruct.cmd_type, "G1") == 0) {
				/*
				snprintf(buffer, 80, "LPC G1 X%d Y%d \r\n", gstruct.x_pos, gstruct.y_pos);
				ITM_write(buffer);
				bresenhamCoord(&motor, motor.getPos(XAXIS), motor.getPos(YAXIS), gstruct.x_pos, gstruct.y_pos);
				 */

				int newPositionX = gstruct.x_pos*stepsPerMMX;
				int newPositionY = gstruct.y_pos*stepsPerMMY;
				snprintf(buffer, 80, "LPC G1 X%d Y%d \r\n", newPositionX, newPositionY);
				ITM_write(buffer);
				bresenham(&motor, motor.getPos(XAXIS), motor.getPos(YAXIS), newPositionX, newPositionY);


				/*
				int absX = 0;
				int absY = 0;
				motor.setDirection(XAXIS, (gstruct.x_pos - motor.getPos(XAXIS))>=0); // if newPositionX is large then move left
				motor.setDirection(YAXIS, (gstruct.y_pos - motor.getPos(YAXIS))>=0); // if newPositionY is large then move down
				absX = round(abs((gstruct.x_pos - motor.getPos(XAXIS))*motor.getScale(XAXIS));
				absY = round(abs((gstruct.y_pos - motor.getPos(YAXIS))*motor.getScale(YAXIS));
				if (absX > 0) {
					motor.move(XAXIS, absX*2, motor.getPPS()); //All motor movement/RIT step must be multiplied by 2
				}
				motor.setPos(XAXIS, gstruct.x_pos);
				//Move motor in Y axis
				if (absY > 0) {
					motor.move(YAXIS, absY*2, motor.getPPS()); //All motor movement/RIT step must be multiplied by 2
				}
				motor.setPos(YAXIS, gstruct.y_pos);
				 */


				// Control pen servo
			} else if(strcmp(gstruct.cmd_type,"M1") == 0){
				penMove(gstruct.pen_pos);
				vTaskDelay(50); // Delay a little bit to avoid pen not move up/down before motor move

				// Control laser power
			} else if (strcmp(gstruct.cmd_type,"M4") == 0){
				setLaserPower(gstruct.laserPower);
				ITM_write("Set laser power \r\n");
				vTaskDelay(5); // Delay a little bit to avoid laser not on/off before motor move
			}
		}
	}
}

int main(void) {

	prvSetupHardware();
	SCT_init();
	ITM_init();

	cmdQueue = xQueueCreate(10, sizeof(Gstruct));


	xTaskCreate(vTaskReceive, "vTaksReceive",
			configMINIMAL_STACK_SIZE + 400, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	xTaskCreate(vTaskMotor, "vTaskMotor",
			configMINIMAL_STACK_SIZE + 350, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(cdc_task, "CDC",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
