#ifndef SHELL_HPP
#define SHELL_HPP
#include <Arduino.h>

namespace Shell {

	enum Command {
		DEBUG = 0,
		INVALID = -1,
	};

	Command evaluate(char* input);
	int split(char* str, char*** dest);


}

#endif /* end of include guard: SHELL_HPP */
