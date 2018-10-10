/*
 * Motor.h
 *
 *  Created on: Oct 3, 2018
 *      Author: Usin
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#define ISLEFTD true
#define MDRAWSCALE 31000

#include "DigitalIoPin.h"
#include "ITM_write.h"
#include <stdint.h>

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
	int64_t currentPosX;
	int64_t currentPosY;
};

#endif /* MOTOR_H_ */

