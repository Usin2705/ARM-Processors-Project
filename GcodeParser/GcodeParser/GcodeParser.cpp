
#include "stdafx.h"
//This is for Visual Studio---------------------------
#pragma warning(disable:4996)
#include <fstream>
#include <string>
#include <iostream>
//----------------------------------------------------

struct Cord {
	float x;
	float y;
	float distance;
};

enum PCode { OK, EXIT, ERROR};

PCode parser(const char * myChar);
Cord getCordFromChar(const char * myChar);

int main()
{
	PCode pcode = OK;
	//This is for Visual Studio---------------------------
	std::string line;
	std::ifstream myFile("gcode01.txt");

	while (getline(myFile, line)) {	
		const char *myChar = line.c_str();
		pcode = parser(myChar);		

		//Break if ERROR
		if (pcode == ERROR) {
			break;
		}

		//Break if EXIT
		if (pcode == EXIT) {
			break;
		}
	}	


	system("pause");
	//----------------------------------------------------
	return 0;
}

/** Parser gcode sent from Uart/file.
*   gcode is stored in const char *, avoid using string so microcontroller can also
*	use this function.
*	
*	Parser return PCode to identity the result
*/
PCode parser(const char * myChar) {
	Cord cord = {0, 0, 0};
	char moveG[] = "G1 ";
	char endM[] = "M4 ";
	//TODO add more case (if we know what the gcode mean)

	//strstr return null if can't found G1 in the string (which mean false)
	//if it can read then it get the cord (x, y) from the data.
	if (strstr(myChar, moveG)) {
		cord = getCordFromChar(myChar);
		return OK;
	} 
	//else if we read the "M4 ' we stop the parser
	else if (strstr(myChar, endM)) {
		return EXIT;
	}
}

/** Trying to get cordinate value from G-code
*	Note that to get here, we already checked the correct G-code start with 'G1 '.
*
*/
Cord getCordFromChar(const char * myChar) {
	Cord cord = { 0, 0, 0};	
	sscanf(myChar, "G1 X%f Y%f A0", &cord.x, &cord.y);		

	//debug in visual studio
	std::cout << cord.x << " " << cord.y << std::endl;

	return cord;
}