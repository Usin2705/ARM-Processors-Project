/*
 * Plotter.h
 *
 *  Created on: Oct 17, 2018
 *      Author: Usin
 */

#ifndef PLOTTER_H_
#define PLOTTER_H_

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

#include "ITM_write.h"
#include "Motor.h"

#define ISLEFTD true

typedef enum {XAXIS , YAXIS} Axis;
typedef enum {Xlimit0, Xlimit1, Ylimit0, Ylimit1} Limit;

void RIT_start(int count, int us);
void SCT_init();

class Plotter {

public:
	Plotter(Motor *myMotorX, Motor *myMotorY);
	void move(Axis axis, int count);
	void motorAcce(Axis axis, int count);

	void setPPS(int PPS);
	int getPPS();

	void setIsMoving(bool moving);
	bool getIsMoving();

	void calibrate();

	Motor* getMotorX();
	Motor* getMotorY();

	virtual ~Plotter();

private:
	bool readLimit(Limit limit);

	Motor *motorX;		//Motor of X axis
	Motor *motorY;		//Motor of Y axis

	bool isMoving;			//if it is true then the motor is moving (and not drawing)
	int motorPPS;			//Pulse per second, delay = 500,000/pps. Maximum without acceleration = 6250 not finalized
};


void penMove(int penPos);
void setLaserPower(int laserPower);


#endif /* PLOTTER_H_ */
