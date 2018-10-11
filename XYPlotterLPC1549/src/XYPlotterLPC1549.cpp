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
#include <math.h>

#define SIMULATOR
//#define PLOTTER1


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
void send_reply(const char* cmd_type){

	//const char* reply_M10 = "M10 XY 380 310 0.00 0.00 A0 B0 H0 S80 U0 D255\r\n";		// reply code for M10 command
#ifdef SIMULATOR
	const char* reply_M10 = "M10 XY 500 500 0.00 0.00 A0 B0 H0 S80 U160 D90\r\n";		// reply code for M10 command
#endif
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
	LPC_INMUX->PINTSEL[0] = 0; 				// select pin 0_0(swY0) as interrupt pin 0
	LPC_INMUX->PINTSEL[1] = 35; 			// select pin 1_3((swY1)(32 + 3 = 35)) as interrupt pin 1
	LPC_INMUX->PINTSEL[2] = 9;				// select pin 0_9 (swX0) as interrupt pin 2
	LPC_INMUX->PINTSEL[3] = 29;				// select pin 0_29 (swX1) as interrupt pin 3

	LPC_GPIO_PIN_INT->SIENF = 7; // enables falling edge interrupts
	LPC_GPIO_PIN_INT->ISEL = 0; // configures pin interrupts as edge sensitive(0)

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
			gstruct_to_send = parser.parse_gcode(gcode_str.c_str());
			send_to_queue(gstruct_to_send);
			send_reply(gstruct_to_send.cmd_type);

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

	penMove(160);	//Move pen up
	setLaserPower(255);	//Turn laser off

#ifdef SIMULATOR
	motor.setPPS(2500);
#endif

	motor.calibrate();
	//moveSquare(&motor);
	//moveRhombus(&motor);
	//moveTrapezoid(&motor);
	int32_t newPositionX = 0;
	int32_t newPositionY = 0;
	interrupt_pins_init();
	char buffer[80] = {'\0'};

#ifdef SIMULATOR
	double umToStepX = (double) motor.getLimDist(XAXIS)/50000;
	double umToStepY = (double) motor.getLimDist(YAXIS)/50000;
	double stepPerUmX = (double) 50000/motor.getLimDist(XAXIS);
	double stepPerUmY = (double) 50000/motor.getLimDist(YAXIS);
#endif


	while(1) {

		if(xQueueReceive(cmdQueue, (void*) &gstruct, portMAX_DELAY)) {
			if (strcmp(gstruct.cmd_type, "G1") == 0) {
				newPositionX = gstruct.x_pos*umToStepX;
				newPositionY = gstruct.y_pos*umToStepY;
				snprintf(buffer, 80, "G1 X%ld Y%ld \r\n", newPositionX, newPositionY);
				ITM_write(buffer);
				bresenham(&motor, motor.getPos(XAXIS), motor.getPos(YAXIS), newPositionX, newPositionY);

				// Control pen servo
			} else if(strcmp(gstruct.cmd_type,"M1") == 0){
				penMove(gstruct.pen_pos);

				// Control laser power
			} else if (strcmp(gstruct.cmd_type,"M4") == 0){
				setLaserPower(gstruct.laserPower);
				ITM_write("Set laser power \r\n");
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
