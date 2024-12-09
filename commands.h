#ifndef __COMMANDS_H__
#define __COMMANDS_H__
/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h> //for NULL
#include "classes.h"

#define MAX_LINE_SIZE 80
#define MAX_ARGS 100


/*=============================================================================
* error definitions
=============================================================================*/
enum ParsingError
{
	INVALID_COMMAND = 0,
	//feel free to add more values here
};

/*=============================================================================
* global functions
=============================================================================*/
// @brief parses the command it gets
// @param a string that contains the unparsed command, an array to store parsed
//  strings
int parseCommand(char*, char**);
int processReturnValue(char* args[MAX_ARGS], int numArgs);
int quit(/*char args[1],*/ int numArgs);
int jobs_update();
// int cd(int numArgs,char* path [MAX_LINE_SIZE]);

#endif //__COMMANDS_H__

