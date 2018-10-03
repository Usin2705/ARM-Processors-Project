/*
 * Motor.h
 *
 *  Created on: Oct 3, 2018
 *      Author: Usin
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#define ISLEFTU true

#include "DigitalIoPin.h"
#include "ITM_write.h"

class Motor {
public:
	Motor();
	virtual ~Motor();
	void setLength(char cord, int length);
	void setDirection(char cord, bool isLeftD);
	void setPos(char cord, int currentPos);
	int getPos(char cord);
	int getLength(char cord);
	bool readLimit(char cord);

private:
	DigitalIoPin swY0;
	DigitalIoPin swY1;
	DigitalIoPin swX0;
	DigitalIoPin swX1;
	DigitalIoPin directionX;
	DigitalIoPin directionY;
	int stepLengthX;
	int stepLengthY;
	int currentPosX;
	int currentPosY;
};

#endif /* MOTOR_H_ */

