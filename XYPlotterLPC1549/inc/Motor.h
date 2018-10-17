/*
 * Motor.h
 *
 *  Created on: Oct 3, 2018
 *      Author: Usin
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include "DigitalIoPin.h"

#define ISLEFTD true
#define PPSLASER 1000
#define PPSDEFAULT 2000
#define PPSMAXCALI 8000
#define PPSMAX 4000


class Motor {

public:

	Motor(int portOrigin, int pinOrigin, int portMax, int pinMax, int portDir, int pinDir);

	void setLimDist(int newDist);
	int getLimDist();

	bool readLimitOrigin();
	bool readLimitMaximum();

	void setDirection(bool isLeftD);

	void setPos(int position);
	int getPos();

	virtual ~Motor();

private:

	DigitalIoPin swOrigin;  //Limit switch at Origin
	DigitalIoPin swMaximum;  //Limit switch at Maximum

	DigitalIoPin direction;	//Direction for motor: Left or Right

	int limDist;			// Step Length of motor

	int currenPos;		//current position of motor, measured in steps
};

#endif /* MOTOR_H_ */
