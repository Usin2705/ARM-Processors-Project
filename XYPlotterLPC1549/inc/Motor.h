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
<<<<<<< HEAD
#include <stdint.h>
=======


#define ISLEFTD true
#define MDRAWSCALE 40000

typedef enum {XAXIS , YAXIS} Axis;
typedef enum {Xlimit0, Xlimit1, Ylimit0, Ylimit1} Limit;

void RIT_start(int count, int us);
void SCT_init();
>>>>>>> feat: added basic code (without bresenham)

class Motor {

public:

	Motor();
<<<<<<< HEAD
	virtual ~Motor();
	void setLength(char cord, int length);
	void setDirection(char cord, bool isLeftD);
	void setPos(char cord, int currentPos);
	int getPos(char cord);
	int getLength(char cord);
	bool readLimit(char cord);
=======

	void setLimDist(Axis axis, int newDist);
	int getLimDist(Axis axis);

	void setDirection(Axis axis, bool isLeftD);

	void setPos(Axis axis, int currentPos);
	int getPos(Axis axis);

	void setPPS(int PPS);
	int getPPS();

	void setRITaxis(Axis axis);
	bool readLimit(Limit limit);
	void calibrate();
>>>>>>> feat: added basic code (without bresenham)

	virtual ~Motor();

private:
<<<<<<< HEAD
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
=======

	DigitalIoPin swY0;	//Limit of Y at 0
	DigitalIoPin swY1;	//Limit of Y at max Y
	DigitalIoPin swX0;  //Limit of X at 0
	DigitalIoPin swX1;  //Limit of Y at max X

	DigitalIoPin directionX;	//Direction for X: Left or Right
	DigitalIoPin directionY;	//Direction for Y: Down or Up

	int limDistX;			// Step Length of X
	int limDistY;			// Step Length of Y

	int64_t currentPosX;		//current position of X
	int64_t currentPosY;		//current position of Y

	int motorPPS;	//Pulse per second, delay = 500,000/pps. Maximum without acceleration = 6250 not finalized
>>>>>>> feat: added basic code (without bresenham)
};

#endif /* MOTOR_H_ */

