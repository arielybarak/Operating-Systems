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

char _line[MAX_LINE_SIZE];
// array that contains internal commands names and their corresponding index

/*=============================================================================
* main handlers
=============================================================================*/
// Reaps a terminated child process (without waiting)
void handleFinishChld(int sig) {
    sigset_t maskSet, oldSet;										 //TODO check that "sigemptyset" is not nececcery (and course's staff are assholes)
	sigfillset(&maskSet);
	sigdelset(&maskSet, SIGINT);					// Unmask Ctrl+C
    sigdelset(&maskSet, SIGTSTP);					// Unmask Ctrl+Z //TODO think again if its not a problem (will the handler continue after SIGINT?)
	sigprocmask(SIG_SETMASK, &maskSet, &oldSet);
	int status;
    pid_t pid = waitpid(-1, &status, WNOHANG) > 0;

	if(WIFEXITED(status)){ 							//WIFEXITED determines if a child exited with exit()
		cout << "child exited with exit() ";
		int exitStatus = WEXITSTATUS(status);
		if(!exitStatus)	//exit status != 0, handle error			//TODO continue errors
			cout << "but with error. CHECK IT NOW!\n";	
		else			//exit status == 0, handle success
			cout << "seccesfully\n";
	}
	job_list.job_remove(pid);
	sigprocmask(SIG_SETMASK, &oldSet, &maskSet);
}

//handler for treating ctrl+c
void handleFGKill(int sig) {
	cout << "I'm SMASHING too important for CTRL+C\n";
}

//handler for treating ctrl+z
void handleFg2Bg(int sig) {
	cout << "I'm SMASHING too important for CTRL+Z\n";
}

void HandleConfigPack(){
	struct sigaction sa = { sa.sa_handler = &handleFinishChld };
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;  							//NOCLDSTOP: Prevents SIGCHLD from being delivered when child processes stop 
    sigaction(SIGCHLD, &sa, nullptr);

	struct sigaction sb = { sb.sa_handler = &handleFGKill };
	sb.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sb, nullptr);

	struct sigaction sc = { sc.sa_handler = &handleFg2Bg };
	sc.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &sc, nullptr);
}



/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char* argv[])
{
	HandleConfigPack();	

	char _cmd[MAX_LINE_SIZE];
	while(1)
	{
		char* args[MAX_ARGS];
		printf("smash > ");
		fgets(_line, MAX_LINE_SIZE, stdin);
		strcpy(_cmd, _line);
		_cmd[strlen(_line) + 1] = '\0';
		//execute command
		int numArgs = parseCommand(_cmd,args);
		int ret_val=processReturnValue(args,numArgs,job_list);

		//initialize buffers for next command
		_line[0] = '\0';
		_cmd[0] = '\0';
	}
	return 0;
}
