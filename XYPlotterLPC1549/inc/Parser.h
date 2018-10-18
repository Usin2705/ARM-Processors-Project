

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
	Gstruct parseGcode(const char* buffer, int penUp, int penDw);
	virtual ~Parser();
};

#endif /* PARSER_H_ */
