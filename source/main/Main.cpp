#include "Application.hpp"
#include "logger.h"


#ifndef TEST

int main(int argc, char* const argv[])
{
	Application application;
	return application.Run(argc, argv);
}

extern void logger_format_message(LoggerLevel level, const char* zone, const char* format, ...)
{
	// TOOD: How should we log?
}

#endif
