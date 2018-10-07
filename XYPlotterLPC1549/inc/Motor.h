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


#define ISLEFTD true
#define MDRAWSCALE 40000

void RIT_start(int count, int us);
void SCT_init();

class Motor {
public:
	Motor();
	virtual ~Motor();
	void setLimDist(char cord, int newDist);
	void setDirection(char cord, bool isLeftD);
	void setPos(char cord, int currentPos);
	void setPPS(int PPS);
	void setRITaxis(char axis);
	int getPos(char cord);
	int getLimDist(char cord);
	int getPPS();
	bool readLimit(char cord);
	void calibrate();

private:
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
};


void penMove(int penPos);
void setLaserPower(int laserPower);

#endif /* MOTOR_H_ */
