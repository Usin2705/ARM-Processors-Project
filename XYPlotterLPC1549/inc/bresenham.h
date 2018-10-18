/*
 * bresenham.c
 *
 *  Created on: Oct 10, 2018
 *      Author: Usin
 */
#ifndef BRESENHAM_H_
#define BRESENHAM_H_

#include "Plotter.h"
#include <math.h>


/* Move the motors to draw
 * DrawCurve only called when drawing curve for Bresenham algorithm
 * It is not used to draw straight line (because it's slow)
 *
 */
void drawCurve(Plotter *plotter, int x0, int y0, int x1, int y1) {
	int absX = abs(x1 - x0);
	int absY= abs(y1 - y0);

	//Move motor in X axis
	if (absX > 0) {
		plotter->move(XAXIS, absX*2);
	}

	//Move motor in Y axis
	if (absY > 0) {
		plotter->move(YAXIS, absY*2);
	}
}

void plotLineLow(Plotter *plotter, int x0, int y0, int x1, int y1) {
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
		drawCurve(plotter, oldX, oldY, x, y);
		oldX = x;
		oldY = y;

		if (D>0) {
			y = y + yi;
			D = D - 2*dx;
		}
		D = D + 2*dy;
	}
}

void plotLineHigh(Plotter *plotter, int x0, int y0, int x1, int y1) {
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
		drawCurve(plotter, oldX, oldY, x, y);
		oldX = x;
		oldY = y;

		if (D>0) {
			x = x + xi;
			D = D - 2*dy;
		}
		D = D + 2*dx;

	}
}

/* Draw using Bresenham algorithm.
 * Input including beginning coordinate x0,y0
 * End coordinate x1, y1
 * Depend on the drawing, motor can also acceleration or not.
 *
 */
void bresenham(Plotter *plotter, int x0, int y0, int x1, int y1, bool isLaser) {

	//Regardless of bresenham or not, the direction is set
	plotter->getMotorX()->setDirection((x1 - x0)>=0); // if x1 is large than x0 then move left
	plotter->getMotorY()->setDirection((y1 - y0)>=0); // if y1 is large than y0 then move down

	//If line is diagonal AND not moving then using Bresenham:
	if ((x0 != x1) && (y0 != y1)&&!plotter->getIsMoving()) {

		//If True, Plot with x distance large than y distance
		if (abs(y1 - y0) < abs(x1 - x0)) {

			if (x0 > x1) {
				plotLineLow(plotter, x1, y1, x0, y0);
			} else {
				plotLineLow(plotter, x0, y0, x1, y1);
			}

			//Plot with y distance large than x distance
		} else {
			if (y0 > y1) {
				plotLineHigh(plotter, x1, y1, x0, y0);
			} else {
				plotLineHigh(plotter, x0, y0, x1, y1);
			}
		}

		//Draw normal straight line without Bresenham:
	} else {
		int absX = abs(x1 - x0);
		int absY= abs(y1 - y0);

		if (isLaser&&!plotter->getIsMoving()) {
			plotter->setPPS(PPSLASER);
			if (absX > 0) {
				plotter->move(XAXIS, absX*2);
			}

			//Move motor in Y axis
			if (absY > 0) {
				plotter->move(YAXIS, absY*2);
			}
		} else {
			//Move motor in X axis
			if (absX > 0) {
				plotter->motorAcce(XAXIS, absX*2);
			}

			//Move motor in Y axis
			if (absY > 0) {
				plotter->motorAcce(YAXIS, absY*2);
			}
		}
	}

	plotter->setPPS(PPSDEFAULT); // come back to default PPS
	plotter->getMotorX()->setPos(x1);
	plotter->getMotorY()->setPos(y1);
	//char buffer[80];
	//snprintf(buffer, 80, "BSH G1 X%d Y%d \r\n", x0 + deltaX, y0 + deltaY);
	//ITM_write(buffer);
}



void moveSquare(Plotter *plotter){
	penMove(0);
	while(1) {
		int x = 300;
		int y = 250;
		bresenham(plotter, 250, 250, x, y, false);
		x = 300;
		y = 300;
		bresenham(plotter, 300, 250, x, y, false);
		x = 250;
		y = 300;
		bresenham(plotter, 300, 300, x, y, false);
		x = 250;
		y = 250;
		bresenham(plotter, 250, 300, x, y, false);
		vTaskDelay(1000);
	}
}

void moveRhombus(Plotter *plotter){
	penMove(0);
	while(1) {
		int x = 300;
		int y = 200;
		bresenham(plotter, 250, 250, x, y, false);
		x = 350;
		y = 250;
		bresenham(plotter, 300, 200, x, y, false);
		x = 300;
		y = 300;
		bresenham(plotter, 350, 250, x, y, false);
		x = 250;
		y = 250;
		bresenham(plotter, 300, 300, x, y, false);
		vTaskDelay(1000);
	}
}

void moveTrapezoid(Plotter *plotter){
	penMove(0);
	while(1) {
		int x = 380;
		int y = 200;
		bresenham(plotter, 250, 250, x, y, false);
		x = 320;
		y = 270;
		bresenham(plotter, 380, 200, x, y, false);
		x = 300;
		y = 330;
		bresenham(plotter, 320, 270, x, y, false);
		x = 250;
		y = 250;
		bresenham(plotter, 300, 330, x, y, false);
		vTaskDelay(1000);
	}
}


#endif
