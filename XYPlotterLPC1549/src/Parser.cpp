
/*
 * Parser.cpp
 */

#include "Parser.h"

Parser::Parser() {

}

// parsers G-code into Gcode structure
Gstruct Parser::parse_gcode(const char* buffer){

	Gstruct gstruct;
	char cmd[3] = {0};

	sscanf(buffer, "%s ", cmd);			// scan for command type of the G-code

	/*M1: set pen position*/
	if(strcmp(cmd, "M1")){

		sscanf(buffer, "M1 %d", &gstruct.pen_pos);
	}
	/*M2: save pen up/down position*/
	if(strcmp(cmd, "M2")){

		sscanf(buffer, "M2 U%d D%d ", &gstruct.pen_up, &gstruct.pen_dw);
	}
	/*M5: save the stepper directions, plot area and plotting speed*/
	if(strcmp(cmd, "M5")){

		sscanf(buffer, "M5 A%d B%d H%d W%d S%d", &gstruct.x_dir, &gstruct.y_dir, &gstruct.plot_area_h, &gstruct.plot_area_w, &gstruct.speed);
	}
	/*M10: Log opening in mDraw*/
	if(strcmp(cmd, "M10") == 0){
	}
	/*M11: Limit Status Query*/
	if(strcmp(cmd, "M11") == 0){

	}
	/*G1: Go to position*/
	if(strcmp(cmd, "G1") == 0){

		sscanf(buffer, "G1 X%f Y%f A%d", &gstruct.x_pos, &gstruct.y_pos, &gstruct.abs);
	}
	strcpy(gstruct.cmd_type, cmd);

	return gstruct;
}

Parser::~Parser() {
}