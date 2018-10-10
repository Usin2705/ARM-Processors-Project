/*
 * Motor.cpp
 *
 *  Created on: Oct 3, 2018
 *      Author: Usin
 */

#include <Motor.h>

Motor::Motor() 	: 	swY0(1,3, DigitalIoPin::pullup, true),
swY1 (0,0, DigitalIoPin::pullup, true),
swX0 (0,9, DigitalIoPin::pullup, true),
swX1 (0,29, DigitalIoPin::pullup, true),
directionX(1,0, DigitalIoPin::output, true),
directionY(0,28, DigitalIoPin::output, true)
{

}

Motor::~Motor() {
	// TODO Auto-generated destructor stub
}

void Motor::setLength(char cord, int length) {
	(cord=='X'? stepLengthX: stepLengthY) = length;

	if (cord == 'X') {
		stepLengthX = length;
	} else {
		stepLengthY = length;
	}
}

int Motor::getLength(char cord) {
	return (cord=='X'?stepLengthX:stepLengthY);
}

void Motor::setDirection(char cord, bool isLeftD) {
	if (cord=='X') {
		directionX.write(isLeftD);
	} else {
		directionY.write(isLeftD);
	}
}

void Motor::setPos(char cord, int currentPos) {
	(cord=='X'?currentPosX:currentPosY) = currentPos;
}

int Motor::getPos(char cord) {
	return (cord=='X'?currentPosX:currentPosY);
}


bool Motor::readLimit(char cord) {
	if (cord=='x') {
		return swX0.read();
	} else if (cord=='X'){
		return swX1.read();
	} else if (cord=='y'){
		return swY0.read();
	} else if (cord=='Y'){
		return swY1.read();
	} else {
		ITM_write("Button read can't regconize the limit char code\r\n");
		return true;
	}
}


