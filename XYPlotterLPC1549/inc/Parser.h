

/* Parser.h
 *
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <string.h>
#include <cstring>
#include "stdio.h"
#include "gstruct.h"

class Parser {

public:
	Parser();
	Gstruct parse_gcode(const char* buffer, int pen_up, int pen_dw);
	virtual ~Parser();
};

#endif /* PARSER_H_ */
