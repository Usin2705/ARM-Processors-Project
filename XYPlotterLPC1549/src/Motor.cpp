/*
 * Motor.cpp
 *
 *  Created on: Oct 3, 2018
 *      Author: Usin
 */

#include <Motor.h>
volatile Axis RITaxis = XAXIS;
volatile uint32_t RIT_count;
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
	LPC_SCTLARGE1->MATCHREL[1].L = 999; // match 1 used for duty cycle (in 10 steps)
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
	static DigitalIoPin bthStepX(0,24, DigitalIoPin::output, true);
	static DigitalIoPin bthStepY(0,27, DigitalIoPin::output, true);

	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	// Tell timer that we have processed the interrupt.
	// Timer then removes the IRQ until next match occurs
	Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
	if(RIT_count > 0) {
		RIT_count--;
		if (RITaxis == XAXIS) {
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

Motor::Motor() 	: 	swY0(0,0, DigitalIoPin::pullup, true),
swY1 (1,3, DigitalIoPin::pullup, true),
swX0 (0,9, DigitalIoPin::pullup, true),
swX1 (0,29, DigitalIoPin::pullup, true),
directionX(1,0, DigitalIoPin::output, true),
directionY(0,28, DigitalIoPin::output, true)
{
	motorPPS = 500000/60; //8333
	//motorPPS = 500000/200; //simulator
	RITaxis = XAXIS;
	penMove(160);	//Move pen up
	setLaserPower(255);	//Turn laser off
}

Motor::~Motor() {
	// TODO Auto-generated destructor stub
}

/* Set the distance of limit (measure by steps)
 * if cord = 'X' then set the distance for X
 * Otherwise set for Y
 *
 */
void Motor::move(Axis axis, int count, int pps) {
	RITaxis = axis;
	RIT_start(count, 500000/pps);
}

/* Set the distance of limit (measure by steps)
 * if cord = 'X' then set the distance for X
 * Otherwise set for Y
 *
 */
void Motor::setLimDist(Axis axis, int length) {
	(axis==XAXIS? limDistX: limDistY) = length;
}

/* Get the distance of limit (measure by steps)
 * if cord = 'X' then get the distance of X
 * Otherwise get distance of Y
 *
 */
int Motor::getLimDist(Axis axis) {
	return (axis==XAXIS?limDistX:limDistY);
}

/* Set the direction of Motor
 * If cord = X, True = left, False = Right
 * If cord = Y, True = Down, False = Up
 *
 */
void Motor::setDirection(Axis axis, bool isLeftD) {
	if (axis==XAXIS) {
		directionX.write(isLeftD);
	} else {
		directionY.write(isLeftD);
	}
}

/* Set the position of the motor (measured by steps)
 * if cord = 'X' then set the position for X
 * Otherwise set for Y
 *
 */
void Motor::setPos(Axis axis, int currentPos) {
	(axis==XAXIS?currentPosX:currentPosY) = currentPos;
}

int Motor::getPos(Axis axis) {
	return (axis==XAXIS?currentPosX:currentPosY);
}

/* Read the limit switch
 * Xlimit0 = limit switch X0 (at 0)
 * Xlimit1 = limit switch X1 (at maximum range of X)
 * Ylimit0 = limit switch y0 (at 0)
 * Ylimit1 = limit switch Y1 (at maximum range of Y)
 *
 */
bool Motor::readLimit(Limit limit) {
	if (limit==Xlimit0) {
		return swX0.read();
	} else if (limit==Xlimit1){
		return swX1.read();
	} else if (limit==Ylimit0){
		return swY0.read();
	} else if (limit==Ylimit1){
		return swY1.read();
	} else {
		ITM_write("Button read can't regconize the limit char code\r\n");
		return true;
	}
}

/* Set the PPS of motor
 * delay = 500000/PPS
 *
 */
void Motor::setPPS(int PPS) {
	motorPPS = PPS;
}

/* Get the PPS of motor
 * delay = 500000/PPS
 *
 */
int Motor::getPPS() {
	return motorPPS;
}

void penMove(int penPos){
	int value = penPos*999/255;
	LPC_SCT0->MATCHREL[1].L = 1000 + value;
}

void setLaserPower(int laserPower){
	int value = laserPower*999/255;
	LPC_SCT1->MATCHREL[1].L = value;
}


void Motor::calibrate() {

	ITM_write("---------------------------    CALIBRATE X  -------------------------\r\n");
	int maxSteps = 0;
	RITaxis = XAXIS;
	bool limitread = readLimit(Xlimit0);
	char buffer[80] = {'\0'};
	for (uint8_t i = 0; i < 2; i++) {
		//Move to left (or down if cord == Y), without counting step
		setDirection(RITaxis, ISLEFTD);
		limitread = readLimit(i==0?Xlimit0:Ylimit0);
		while (!limitread){
			limitread = readLimit(i==0?Xlimit0:Ylimit0);
			RIT_start(5, 500000/motorPPS);
		}

		//	When move back to right (or up if cord == Y), count step
		setDirection(RITaxis, !ISLEFTD);
		limitread = readLimit(i==0?Xlimit1:Ylimit1);
		while (!limitread){
			limitread = readLimit(i==0?Xlimit1:Ylimit1);
			RIT_start(5,500000/motorPPS);
			maxSteps =  maxSteps + 2; //last RIT step doesn't  count
		}
		setLimDist(RITaxis, maxSteps);
		setPos(RITaxis, 0);
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
	setDirection(XAXIS, ISLEFTD);
	setDirection(YAXIS, ISLEFTD);
	RITaxis = XAXIS;
	RIT_start(getLimDist(RITaxis),500000/motorPPS); //All motor movement/RIT step must be multiplied by 2
	RITaxis = YAXIS;
	RIT_start(getLimDist(RITaxis),500000/motorPPS); //All motor movement/RIT step must be multiplied by 2

	setPos(XAXIS, (getLimDist(XAXIS)/2)-1); // Set position in scale with mDraw
	setPos(YAXIS, (getLimDist(YAXIS)/2)-1); // Set position in scale with mDraw
}


