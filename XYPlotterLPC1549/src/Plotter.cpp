/*
 * Plotter.cpp
 *
 *  Created on: Oct 17, 2018
 *      Author: Usin
 */

#include <Plotter.h>
volatile Axis RITaxis = XAXIS;
volatile uint32_t RIT_count;
volatile bool isCalibrate = true;
xSemaphoreHandle sbRIT = xSemaphoreCreateBinary();

void SCT_init(){
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
	LPC_SCTLARGE1->MATCHREL[1].L = 0; // match 1 used for duty cycle (in 10 steps)
	LPC_SCTLARGE1->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCTLARGE1->EVENT[0].CTRL = (1 << 12); // match 0 condition only
	LPC_SCTLARGE1->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
	LPC_SCTLARGE1->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only
	LPC_SCTLARGE1->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCTLARGE1->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0
	LPC_SCTLARGE1->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg
}

extern "C" {

void RIT_IRQHandler(void) {
	static DigitalIoPin btnStepX(0,24, DigitalIoPin::output, true);
	static DigitalIoPin btnStepY(0,27, DigitalIoPin::output, true);

	DigitalIoPin swY0(0,0, DigitalIoPin::pullup, true);
	DigitalIoPin swY1(1,3, DigitalIoPin::pullup, true);
	DigitalIoPin swX0(0,9, DigitalIoPin::pullup, true);
	DigitalIoPin swX1(0,29, DigitalIoPin::pullup, true);

	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	// Tell timer that we have processed the interrupt.
	// Timer then removes the IRQ until next match occurs
	Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
	if(RIT_count > 0) {
		/*
		if (RITaxis == XAXIS) {
			isHighX = !(bool)isHighX;
			btnStepX.write(isHighX);
		} else {
			isHighY = !(bool)isHighY;
			btnStepY.write(isHighY);
		}*/

		if (isCalibrate||(!swX0.read() && !swX1.read() && !swY0.read() && !swY1.read())) {

			if (RITaxis == XAXIS) {
				btnStepX.write(!btnStepX.read());
			} else {
				btnStepY.write(!btnStepY.read());
			}

			RIT_count--;
		}
	}

	if (RIT_count <= 0) {
		Chip_RIT_Disable(LPC_RITIMER); // disable timer
		// Give semaphore and set context switch flag if a higher priority task was woken up
		xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
	}
	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}
/* All move must call Motor.setRITaxis(Axis axis) to set which motor to move
 *
 */
void RIT_start(int count, int us) {
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

Plotter::Plotter(Motor *myMotorX, Motor *myMotorY)
{
	motorX = myMotorX;
	motorY = myMotorY;
}

Plotter::~Plotter() {
	// TODO Auto-generated destructor stub
}

/* Set the distance of limit (measure by steps)
 * if cord = 'X' then set the distance for X
 * Otherwise set for Y
 *
 */
void Plotter::move(Axis axis, int count) {
	RITaxis = axis;
	RIT_start(count, 500000/motorPPS);
}

/* Accelerate the motor
 * then decelerate (to avoid moving too fast)
 * Depend on the distance, if long enough it can move with PPSMAXCALI
 * Using PPSMAXCALI require acceleration and deceleration
 * If the distance is short then it only move with PPSMAX
 * Using PPSMAX only require deceleration
 *
 */
void Plotter::motorAcce(Axis axis, int count) {

	//If moving long distance then accelerate then decelerate
	// 5500 counts = 2750 steps = ~3.1cm
	if (count > 5500) {
		//acceleration
		// 2000 counts
		while (count > 3500) {
			move(axis, 10);
			count = count - 10;
			motorPPS = motorPPS + (PPSMAXCALI-PPSDEFAULT)/200;
			if (motorPPS > PPSMAXCALI)  {
				motorPPS = PPSMAXCALI;
				break;
			}
		}

		if (count > 2000) {
			motorPPS = PPSMAXCALI;
			move(axis, count - 2000);
			count = 2000;
		}

		//Deceleration
		//2000 counts
		while (count > 0) {
			move(axis, 10);
			count = count - 10;
			motorPPS = motorPPS - (PPSMAXCALI-PPSDEFAULT)/200;
			if (motorPPS < PPSDEFAULT) {
				motorPPS = PPSDEFAULT;
			}
		}
		// If moving short distant then don't need to accelerate, however using lower speed
	} else {
		if (count > 20) {
			motorPPS = PPSMAX;
			move(axis, count - 20);
			count = 20;
		}

		while (count > 0) {
			motorPPS = motorPPS - (PPSMAX-PPSDEFAULT)/10;
			if (motorPPS < PPSDEFAULT) {
				motorPPS = PPSDEFAULT;
			}
			move(axis, 2);
			count = count - 2;
		}
	}

	motorPPS = PPSDEFAULT;
}

/* Read the limit switch
 * Xlimit0 = limit switch X0 (at 0)
 * Xlimit1 = limit switch X1 (at maximum range of X)
 * Ylimit0 = limit switch y0 (at 0)
 * Ylimit1 = limit switch Y1 (at maximum range of Y)
 *
 */
bool Plotter::readLimit(Limit limit) {
	if (limit==Xlimit0) {
		return motorX->readLimitOrigin();
	} else if (limit==Xlimit1){
		return motorX->readLimitMaximum();
	} else if (limit==Ylimit0){
		return motorY->readLimitOrigin();
	} else if (limit==Ylimit1){
		return motorY->readLimitMaximum();
	} else {
		ITM_write("Button read can't regconize the limit char code\r\n");
		return true;
	}
}

/* Set the PPS of motors
 * delay = 500000/PPS
 *
 */
void Plotter::setPPS(int PPS) {
	motorPPS = PPS;
}

/* Get the PPS of motors
 * delay = 500000/PPS
 *
 */
int Plotter::getPPS() {
	return motorPPS;
}

/* Set the moving boolean of the motors
 * if it is true then the motor is moving (and not drawing)
 * then we can acceleration it a little bit (too much would make the drawing bad)
 *
 */
void Plotter::setIsMoving(bool moving) {
	isMoving = moving;
}

/* Get the moving boolean of the motor
 * if it is true then the motor is moving (and not drawing)
 * then we can acceleration it a little bit (too much would make the drawing bad)
 *
 */
bool Plotter::getIsMoving() {
	return isMoving;
}

/*
 * Return the motor X
 */
Motor* Plotter::getMotorX() {
	return motorX;
}

/*
 * Return the motor Y
 */
Motor* Plotter::getMotorY() {
	return motorY;
}

/*	Move the pen
 * 	(scale to 1000)
 *
 */
void penMove(int penPos){
	int value = penPos*999/255;
	LPC_SCT0->MATCHREL[1].L = 1000 + value;
}

/* Set laser power
 * (scale to 1000)
 *
 */
void setLaserPower(int laserPower){
	int value = laserPower*999/255;
	LPC_SCT1->MATCHREL[1].L = value;
}


void Plotter::calibrate() {

	ITM_write("---------------------------    CALIBRATE X  -------------------------\r\n");
	int maxSteps = 0;
	RITaxis = XAXIS;
	bool limitread = readLimit(Xlimit0);
	char buffer[80] = {'\0'};
	int step = 8;
	for (uint8_t i = 0; i < 2; i++) {
		//Move to left (or down if cord == Y), without counting step

		(RITaxis==XAXIS?motorX:motorY)->setDirection(ISLEFTD);
		limitread = readLimit(RITaxis==XAXIS?Xlimit0:Ylimit0);
		while (!limitread){
			limitread = readLimit(RITaxis==XAXIS?Xlimit0:Ylimit0);
			motorPPS = PPSMAX;
			RIT_start(step, 500000/motorPPS);
		}

		//	When move back to right (or up if cord == Y), count step
		(RITaxis==XAXIS?motorX:motorY)->setDirection(!ISLEFTD);
		limitread = readLimit(RITaxis==XAXIS?Xlimit1:Ylimit1);
		motorPPS = PPSDEFAULT;
		while (!limitread){
			limitread = readLimit(RITaxis==XAXIS?Xlimit1:Ylimit1);

			//Acceleration
			if (maxSteps < 5000) {
				motorPPS += 500;
				motorPPS = motorPPS>PPSMAXCALI?PPSMAXCALI:motorPPS;
				step = 1000;

				//If within moving distance (not hit the limit soon) move really fast
			} else if (maxSteps < 23000){
				motorPPS = PPSMAXCALI;
				step = 1000;

				//Else decelerate until hit the limit
			} else {
				step = 8;
				motorPPS = motorPPS>PPSDEFAULT?motorPPS-5:PPSDEFAULT;
			}
			RIT_start(step,500000/motorPPS);
			maxSteps= maxSteps + step/2;
		}

		(RITaxis==XAXIS?motorX:motorY)->setLimDist(maxSteps);
		snprintf(buffer, 80, "Max steps of axis %d is: %d \r\n", RITaxis, maxSteps);
		ITM_write(buffer);
		//If 'X' then change to Y, do the loop again
		if (RITaxis == XAXIS) {
			ITM_write("---------------------------    CALIBRATE Y  -------------------------\r\n");
		}
		maxSteps = 0;
		RITaxis = YAXIS;
	}

	//Move to middle
	motorX->setDirection(ISLEFTD);
	motorY->setDirection(ISLEFTD);

	vTaskDelay(500);
	RITaxis = XAXIS;
	RIT_start(500 ,500000/motorPPS);
	RITaxis = YAXIS;
	RIT_start(500 ,500000/motorPPS);

	motorPPS = PPSDEFAULT;
	motorX->setPos(0); // Set position in scale with mDraw
	motorY->setPos(0); // Set position in scale with mDraw
	isMoving = true;
	isCalibrate = false;
}



