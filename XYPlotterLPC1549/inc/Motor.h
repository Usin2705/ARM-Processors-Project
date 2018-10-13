/*
 * Motor.h
 *
 *  Created on: Oct 3, 2018
 *      Author: Usin
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <mutex>

#include "DigitalIoPin.h"
#include <stdint.h>
#include "ITM_write.h"


#define SIMULATOR
#define PLOTTER1


#define ISLEFTD true

typedef enum {XAXIS , YAXIS} Axis;
typedef enum {Xlimit0, Xlimit1, Ylimit0, Ylimit1} Limit;

void RIT_start(int count, int us);
void SCT_init();

class Motor {

public:

	Motor();
	void move(Axis axis, int count, int pps);

	void setLimDist(Axis axis, int newDist);
	int getLimDist(Axis axis);

	void setDirection(Axis axis, bool isLeftD);

	void setPos(Axis axis, int currentPos);
	int getPos(Axis axis);

	void setScale(Axis axis, double stepsPerMM);
	double getScale(Axis axis);

	void setPPS(int PPS);
	int getPPS();

	bool readLimit(Limit limit);
	void calibrate();

	virtual ~Motor();

private:

	DigitalIoPin swY0;	//Limit of Y at 0
	DigitalIoPin swY1;	//Limit of Y at max Y
	DigitalIoPin swX0;  //Limit of X at 0
	DigitalIoPin swX1;  //Limit of Y at max X

	DigitalIoPin directionX;	//Direction for X: Left or Right
	DigitalIoPin directionY;	//Direction for Y: Down or Up

	int limDistX;			// Step Length of X
	int limDistY;			// Step Length of Y

	int currentCoordX;		//current position of X, measured in xcoord in mDraw
	int currentCoordY;		//current position of Y, measured in ycoord in mDraw

	double stepsPerMMX; 	//to convert from xcoord to steps
	double stepsPerMMY;		//to convert from ycoord to steps


	int motorPPS;	//Pulse per second, delay = 500,000/pps. Maximum without acceleration = 6250 not finalized
};


void penMove(int penPos);
void setLaserPower(int laserPower);

#endif /* MOTOR_H_ */
