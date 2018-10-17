/*
 * Motor.cpp
 *
 *  Created on: Oct 3, 2018
 *      Author: Usin
 */

#include <Motor.h>

Motor::Motor(int portOrigin, int pinOrigin, int portMax, int pinMax, int portDir, int pinDir):
		swOrigin(portOrigin, pinOrigin, DigitalIoPin::pullup, true),
		swMaximum (portMax, pinMax, DigitalIoPin::pullup, true),
		direction(portDir, pinDir, DigitalIoPin::output, true)
{

}

Motor::~Motor() {
	// TODO Auto-generated destructor stub
}


/* Set the distance of limit (measure by steps)
 *
 */
void Motor::setLimDist(int length) {
	limDist = length;
}

/* Get the distance of limit (measure by steps)
 *
 */
int Motor::getLimDist() {
	return limDist;
}

/*
 * Read the limit switch at origin
 */
bool Motor::readLimitOrigin () {
	return swOrigin.read();
}

/*
 * Read the limit switch at maximum
 */
bool Motor::readLimitMaximum () {
	return swMaximum.read();
}

/* Set the direction of Motor
 * If cord = X, True = left, False = Right
 * If cord = Y, True = Down, False = Up
 *
 */
void Motor::setDirection(bool isLeftD) {
	direction.write(isLeftD);
}

/* Set the position of the motor (measure by steps from mDraw)
 *
 */
void Motor::setPos(int position) {
	currenPos = position;
}

/* Return the position of the motor (measure by steps)
 */
int Motor::getPos() {
	return currenPos;
}


