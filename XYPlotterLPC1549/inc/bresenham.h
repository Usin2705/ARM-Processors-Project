/*
 * bresenham.c
 *
 *  Created on: Oct 10, 2018
 *      Author: Usin
 */
#ifndef BRESENHAM_H_
#define BRESENHAM_H_

#include "Motor.h"
#include <math.h>

void drawplot(Motor *motor, int x0, int y0, int x1, int y1) {
	int absX = abs(x1 - x0);
	int absY= abs(y1 - y0);

	//Move motor in X axis
	if (absX > 0) {
		motor->move(XAXIS, absX*2);
	}

	//Move motor in Y axis
	if (absY > 0) {
		motor->move(YAXIS, absY*2);
	}
}

void plotLineLow(Motor *motor, int x0, int y0, int x1, int y1) {
	int dx = x1 - x0;
	int dy = y1 - y0;
	int yi = 1;
	if (dy < 0) {
		yi = -1;
		dy = -dy;
	}

	int D = 2*dy - dx;
	int y = y0;
	int oldX = x0;
	int oldY = y0;
	for (int x = x0; x <= x1; x++) {
		drawplot(motor, oldX, oldY, x, y);
		oldX = x;
		oldY = y;

		if (D>0) {
			y = y + yi;
			D = D - 2*dx;
		}
		D = D + 2*dy;
	}
}

void plotLineHigh(Motor *motor, int x0, int y0, int x1, int y1) {
	int dx = x1 - x0;
	int dy = y1 - y0;
	int xi = 1;
	if (dx<0) {
		xi = -1;
		dx = -dx;
	}

	int D = 2*dx - dy;
	int x = x0;
	int oldX = x0;
	int oldY = y0;
	for (int y = y0; y <= y1; y++) {
		drawplot(motor, oldX, oldY, x, y);
		oldX = x;
		oldY = y;

		if (D>0) {
			x = x + xi;
			D = D - 2*dy;
		}
		D = D + 2*dx;

	}
}

void bresenham(Motor *motor, int x0, int y0, int x1, int y1, bool isLaser) {
	int deltaX = x1 - x0;
	int deltaY = y1 - y0;

	//Regardless of bresenham or not, the direction is set
	motor->setDirection(XAXIS, deltaX>=0); // if newPositionX is large then move left
	motor->setDirection(YAXIS, deltaY>=0); // if newPositionY is large then move down

	//If line is diagonal AND not moving then using Bresenham:
	if ((x0 != x1) && (y0 != y1)&&!motor->getIsMoving()) {

		//If True, Plot with x distance large than y distance
		if (abs(y1 - y0) < abs(x1 - x0)) {

			if (x0 > x1) {
				plotLineLow(motor, x1, y1, x0, y0);
			} else {
				plotLineLow(motor, x0, y0, x1, y1);
			}

			//Plot with y distance large than x distance
		} else {
			if (y0 > y1) {
				plotLineHigh(motor, x1, y1, x0, y0);
			} else {
				plotLineHigh(motor, x0, y0, x1, y1);
			}
		}

		//Draw normal straight line without Bresenham:
	} else {
		int absX = abs(x1 - x0);
		int absY= abs(y1 - y0);

		if (isLaser&&!motor->getIsMoving()) {
			motor->setPPS(PPSLASER);
			if (absX > 0) {
				motor->move(XAXIS, absX*2);
			}

			//Move motor in Y axis
			if (absY > 0) {
				motor->move(YAXIS, absY*2);
			}
		} else {
			//Move motor in X axis
			if (absX > 0) {
				motor->motorAcce(XAXIS, absX*2);
			}

			//Move motor in Y axis
			if (absY > 0) {
				motor->motorAcce(YAXIS, absY*2);
			}
		}
	}

	motor->setPPS(PPSDEFAULT); // come back to default PPS
	motor->setPos(XAXIS, x0 + deltaX);
	motor->setPos(YAXIS, y0 + deltaY);
	char buffer[80];
	snprintf(buffer, 80, "BSH G1 X%d Y%d \r\n", x0 + deltaX, y0 + deltaY);
	ITM_write(buffer);
}



void moveSquare(Motor *motor){
	penMove(0);
	while(1) {
		int x = 300;
		int y = 250;
		bresenham(motor, 250, 250, x, y, false);
		x = 300;
		y = 300;
		bresenham(motor, 300, 250, x, y, false);
		x = 250;
		y = 300;
		bresenham(motor, 300, 300, x, y, false);
		x = 250;
		y = 250;
		bresenham(motor, 250, 300, x, y, false);
		vTaskDelay(1000);
	}
}

void moveRhombus(Motor *motor){
	penMove(0);
	while(1) {
		int x = 300;
		int y = 200;
		bresenham(motor, 250, 250, x, y, false);
		x = 350;
		y = 250;
		bresenham(motor, 300, 200, x, y, false);
		x = 300;
		y = 300;
		bresenham(motor, 350, 250, x, y, false);
		x = 250;
		y = 250;
		bresenham(motor, 300, 300, x, y, false);
		vTaskDelay(1000);
	}
}

void moveTrapezoid(Motor *motor){
	penMove(0);
	while(1) {
		int x = 380;
		int y = 200;
		bresenham(motor, 250, 250, x, y, false);
		x = 320;
		y = 270;
		bresenham(motor, 380, 200, x, y, false);
		x = 300;
		y = 330;
		bresenham(motor, 320, 270, x, y, false);
		x = 250;
		y = 250;
		bresenham(motor, 300, 330, x, y, false);
		vTaskDelay(1000);
	}
}


#endif
