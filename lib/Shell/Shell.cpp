#include "Shell.h"
#include <stdio.h>
#include <string.h>


Shell::Command Shell::evaluate(char* input) {
	char** args;
	int argc = Shell::split(input, &args);
	if (argc <= 0) {
		delete args;
		return Shell::Command::INVALID;
	}
	Serial.print("Command: ");
	Serial.println(args[0]);
	Serial.flush();
	if (strncmp(args[0], "debug", 5) == 0) {
		delete args;
		return Shell::Command::DEBUG;
	}
	if (strncmp(args[0], "test", 4) == 0) {
		delete args;
		return Shell::Command::TEST;
	}
	delete args;
	return Shell::Command::INVALID;
}

int Shell::split(char* str, char*** dest) {
	int len = strlen(str);
	if (len <= 0) return 0;
	int counter = 1;
	for (int i = 0; i < len; ++ i) {
		if (str[i] == ' ') counter++;
	}
	*dest = new char*[counter];
	for (int i = 0; i < counter; ++i) {
		*dest[i] = strtok(str, " ");
	}
	return counter;
}
