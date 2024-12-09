//smash.c

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <string.h>

#include "classes.h"
#include "commands.h"
#include "signals.h"
#include <sys/types.h>
#include <sys/wait.h>

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
job_arr job_list;
char _line[MAX_LINE_SIZE];
// array that contains internal commands names and their corresponding index



/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char* argv[])
{
	MainHandleConfigPack();	
	char _cmd[MAX_LINE_SIZE];

	while(1)
	{
		// jobs_update();
		char* args[MAX_ARGS];
		printf("smash > ");
		fgets(_line, MAX_LINE_SIZE, stdin);
		strcpy(_cmd, _line);
		_cmd[strlen(_line) + 1] = '\0';
		//execute command
		int numArgs = parseCommand(_cmd,args);
		int ret_val=processReturnValue(args,numArgs);
		//initialize buffers for next command
		_line[0] = '\0';
		_cmd[0] = '\0';
	}
	return 0;
}
