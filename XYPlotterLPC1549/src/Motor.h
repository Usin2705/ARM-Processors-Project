
/*
 * Motor:
 * Motor Class is class for stepper motor, that is resposible for moving around.
 *
 *
 * Motor Class Definitions
 *
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#define MDRAWSCALE 31000
#include "DigitalIoPin.h"
#include "ITM_write.h"
#include <stdint.h>

class Motor {

public:

	Motor();

	void setLimDistX(int newdist);
	void setLimDistY(int newdist);
	int getLimDistX();
	int getLimDistY();

	void setDirX(bool right);
	void setDirY(bool up);
	int getDirX();
	int getDirY();

	void setPosX(int newposX);
	void setPosY(int newposY);
	int64_t getPosX();
	int64_t getPosY();

	bool readLimitX0();
	bool readLimitX1();
	bool readLimitY0();
	bool readLimitY1();

	virtual ~Motor();

private:

	DigitalIoPin swY0;
	DigitalIoPin swY1;
	DigitalIoPin swX0;
	DigitalIoPin swX1;
	DigitalIoPin directionX;
	DigitalIoPin directionY;

	int limDistX;
	int limDistY;

	int64_t currentPosX;
	int64_t currentPosY;
};

#endif /* MOTOR_H_ */
