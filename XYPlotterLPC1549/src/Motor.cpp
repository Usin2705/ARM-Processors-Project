
/*
 * Motor Class Implementation
 */

#include "Motor.h"

Motor::Motor() 	: 	swY0(1,3, DigitalIoPin::pullup, true),
swY1 (0,0, DigitalIoPin::pullup, true),
swX0 (0,9, DigitalIoPin::pullup, true),
swX1 (0,29, DigitalIoPin::pullup, true),
directionX(1,0, DigitalIoPin::output, true),
directionY(0,28, DigitalIoPin::output, true)
{

	limDistX = 0;
	limDistY = 0;
	currentPosX = 0;
	currentPosY = 0;
}

// set distance between plotters x-axis limits
void Motor::setLimDistX(int newdist){

	limDistX = newdist;
}
// set distance between plotters y-axis limits
void Motor::setLimDistY(int newdist){

	limDistY = newdist;
}

// get distance between x-axis limits
int Motor::getLimDistX(){

	return limDistX;
};
// get distance between y-axis limits
int Motor::getLimDistY(){

	return limDistY;
};

// set the X direction: righ/down
void Motor::setDirX(bool right){

	directionX.write(right);
}
//set the Y direction: up/down
void Motor::setDirY(bool up){

	directionY.write(up);
}

// sets new x position for the motor
void Motor::setPosX(int newposX ){

	currentPosX = newposX;
}
// sets new y position for the motor
void Motor::setPosY(int newposY ){

	currentPosY = newposY;
}

// get current x position
int64_t Motor::getPosX(){

	return currentPosX;
}
// get current y position
int64_t Motor::getPosY(){

	return currentPosY;
}

// if limit switch X0 is hit
bool Motor::readLimitX0(){

	return swX0.read();
}
// if limit switch X1 is hit
bool Motor::readLimitX1(){

	return swX1.read();
}

// if limit switch Y0 is hit
bool Motor::readLimitY0(){

	return swY0.read();
}
// if limit switch Y1 is hit
bool Motor::readLimitY1(){

	return swY1.read();
}

// default destructor
Motor::~Motor() {

}
