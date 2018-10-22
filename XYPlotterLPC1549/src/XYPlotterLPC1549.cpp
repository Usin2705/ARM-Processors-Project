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

// Queue for gcode command structs
QueueHandle_t cmdQueue;


int penUp = 0;
int penDown = 0;

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
	const char* replyOK = "OK\r\n";


	/*
	std::string replyM10 = "";
	#if defined(PLOTTER1)
	replyM10 = "M10 XY 310 345 0.00 0.00 A0 B0 H0 S80 U0 D180\r\n";		// reply code for M10 command
	#elif defined(PLOTTER2)
	replyM10 =	"M10 XY 310 340 0.00 0.00 A0 B0 H0 S80 U0 D180\r\n";		// reply code for M10 command
	#elif defined(PLOTTER3)
	replyM10 =	"M10 XY 305 345 0.00 0.00 A0 B0 H0 S80 U80 D0\r\n";		// reply code for M10 command
	#else
	replyM10 =	"M10 XY 500 500 0.00 0.00 A0 B0 H0 S80 U160 D90\r\n";
	#endif
	 */

	if(strcmp(g->cmd_type, "M10") == 0){
		char replyM10[100] = {0};

#if defined(PLOTTER1)
		sprintf(replyM10, "M10 XY 310 345 0.00 0.00 A0 B0 H0 S50 U%d D%d\r\nOK\r\n", g->pen_up, g->pen_dw);
#elif defined(PLOTTER2)
		sprintf(replyM10, "M10 XY 310 340 0.00 0.00 A0 B0 H0 S50 U%d D%d\r\nOK\r\n", g->pen_up, g->pen_dw);
#elif defined(PLOTTER3)
		sprintf(replyM10, "M10 XY 305 345 0.00 0.00 A0 B0 H0 S50 U%d D%d\r\nOK\r\n", g->pen_up, g->pen_dw);
#else
		sprintf(replyM10, "M10 XY 500 500 0.00 0.00 A0 B0 H0 S50 U%d D%d\r\nOK\r\n", g->pen_up, g->pen_dw);
#endif


		ITM_write(replyM10);
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

			gstruct_to_send = parser.parseGcode(gcode_str.c_str(), penUp, penDown);
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
	Motor motorX(0, 9, 0, 29, 1, 0);
	Motor motorY(0, 0, 1, 3, 0, 28);
	Plotter plotter(&motorX, &motorY);

	setLaserPower(0);	//Turn laser off
	plotter.setPPS(PPSDEFAULT);
	bool isPenCalibrated = true;	//Check if the pen is calibrated or not

#if defined(PLOTTER1)
	penUp = 0;
	penDown = 180;
	penMove(penUp); 		//Move pen up
#elif defined(PLOTTER2)
	penUp = 0;
	penDown = 180;
	penMove(penUp); 		//Move pen up
#elif defined(PLOTTER3)
	penUp = 80;
	penDown = 0;
	penMove(penUp); 			//Move pen up
#else
	penMove(160);			//Move pen up
	isPenCalibrated = false;
#endif

	//For calibrate the pen
	while (!isPenCalibrated) {
		if(xQueueReceive(cmdQueue, (void*) &gstruct, portMAX_DELAY)) {
			if (strcmp(gstruct.cmd_type,"M2") == 0){
				penUp = gstruct.pen_up;
				penDown = gstruct.pen_dw;

			} else if(strcmp(gstruct.cmd_type,"M1") == 0){
				penMove(gstruct.pen_pos);
				vTaskDelay(50); // Delay a little bit to avoid pen not move up/down before motor move

			} else if (strcmp(gstruct.cmd_type,"M28") == 0){
				isPenCalibrated = true;
			}
		}
	}

	plotter.calibrate();
	//moveSquare(&plotter);
	//moveRhombus(&plotter);
	//moveTrapezoid(&plotter);

	double stepsPerMMX;
	double stepsPerMMY;
	bool isLaser = false;


#if defined(PLOTTER1)
	stepsPerMMX = (double) plotter.getMotorX()->getLimDist()/31000.0; //31cm
	stepsPerMMY = (double) plotter.getMotorY()->getLimDist()/34500.0; //34.5cm
#elif defined(PLOTTER2)
	stepsPerMMX = (double) plotter.getMotorX()->getLimDist()/31000.0; //31cm
	stepsPerMMY = (double) plotter.getMotorY()->getLimDist()/34000.0; //34cm
#elif defined(PLOTTER3)
	stepsPerMMX = (double) plotter.getMotorX()->getLimDist()/30500.0; //30.5cm
	stepsPerMMY = (double) plotter.getMotorY()->getLimDist()/34500.0; //34.5cm
#else
	stepsPerMMX = (double) plotter.getMotorX()->getLimDist()/50000.0;
	stepsPerMMY = (double) plotter.getMotorY()->getLimDist()/50000.0;
#endif

	//char buffer[80] = {'\0'};
	while(1) {

		if(xQueueReceive(cmdQueue, (void*) &gstruct, portMAX_DELAY)) {
			if (strcmp(gstruct.cmd_type, "G1") == 0) {

				int newPositionX = gstruct.x_pos*stepsPerMMX;
				int newPositionY = gstruct.y_pos*stepsPerMMY;
				//snprintf(buffer, 80, "LPC G1 X%d Y%d \r\n", newPositionX, newPositionY);
				//ITM_write(buffer);
				bresenham(&plotter, plotter.getMotorX()->getPos(), plotter.getMotorY()->getPos(),
						newPositionX, newPositionY, isLaser);

				// Control pen servo
			} else if(strcmp(gstruct.cmd_type,"M1") == 0){
				penMove(gstruct.pen_pos);
				plotter.setIsMoving(gstruct.pen_pos==penUp); //If pen up then move and not draw
				vTaskDelay(200); // Delay a little bit to avoid pen not move up/down before motor move


				// Control laser power
			} else if (strcmp(gstruct.cmd_type,"M4") == 0){
				vTaskDelay(5); // Delay a little bit to avoid laser not on/off before motor move
				setLaserPower(gstruct.laserPower);

				if (gstruct.laserPower>0) {
					isLaser = true;
				}

				if (isLaser) {
					plotter.setIsMoving(gstruct.laserPower==0);
				}
				ITM_write("Set laser power \r\n");
				vTaskDelay(5); // Delay a little bit to avoid laser not on/off before motor move

			// if stop and start again then turn off laser, move pen up, set isMoving = true
			} else if(strcmp(gstruct.cmd_type,"M10") == 0){
				isLaser = false;
				penMove(penUp);
				setLaserPower(0);
				plotter.setIsMoving(true);
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
