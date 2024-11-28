//smash.c

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>
#include "commands.h"
#include "signals.h"
#include <iostream>
#include <time.h>
#include <classes.cpp>

using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::prev;

#define FG   '1'
#define BG 	 '2'
#define STOPPED '3'


/*=============================================================================
* classes/structs declarations
=============================================================================*/
/*=============================================================================
* global variables & data structures
=============================================================================*/

char _line[MAX_LINE_SIZE];
char* args[MAX_ARGS];
const char* commands[9] = {"showpid","pwd","jobs","kill","fg","bg",
															"quit",	"diff"};


/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char* argv[])
{
	char* args[MAX_ARGS];
	job_arr job_array ;
	char _cmd[MAX_LINE_SIZE];
	while(1)
	{
		printf("smash > ");
		fgets(_line, MAX_LINE_SIZE, stdin);
		strcpy(_cmd, _line);
		_cmd[strlen(_line) + 1] = '\0';
		//execute command
		int numArgs = parseCommandExample(_cmd);


		//initialize buffers for next command
		_line[0] = '\0';
		_cmd[0] = '\0';
	}

	return 0;
}
