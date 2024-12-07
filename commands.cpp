//commands.cpp
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <cstring>


#include "signals.h"
#include "commands.h"
#include "classes.h"

using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::prev;

const char* commands[9] = {"showpid","pwd","cd","jobs","kill","fg","bg",
															"quit","diff"};
extern job_arr job_list;
//example function for parsing commands

int run_command(int op);
int identify_cmd(char* cmd);
int showpid (int numArgs);
int pwd(int numArgs);
// int cd(int numArgs,char* path [MAX_LINE_SIZE]);
int quit(/*char* args[1],*/ int numArgs);

void func_handleFg2Bg(int sig);
void func_handleFg2Bg(int sig);
void func_HandleConfigPack(); 	//PUT IN ALL HOMEMADE (LOL) FUNCTIONS


int parseCommand(char* line, char* args[])						//TODO I guess we need masking for both functions
{
	char delimiters[]=" \t\n";//parsing should be done by spaces, tabs or newlines
	int numArgs = 0;
	args[numArgs] = strtok(line, delimiters); //read strtok documentation - parses string by delimiters
	if(!args[0])
		return INVALID_COMMAND; //this means no tokens were found, most like since command is invalid
		
	while(numArgs < MAX_ARGS)
	{
		args[++numArgs] = strtok(NULL, delimiters); //first arg NULL -> keep tokenizing from previous call
		if(!args[numArgs]){
			numArgs--;
			break;
		}
	}
	/*
	At this point cmd contains the command string and the args array contains
	the arguments. You can return them via struct/class, for example in C:
		typedf struct
		{
			char* cmd;
			char* args[MAX_ARGS];
		} Command;
	Or continue the execution from here.
	*/
	return numArgs;
}

//example function for getting/setting return value of a process.

// @brief process the given command and run it
// @param args array, number of args job array

int processReturnValue(char* args[MAX_ARGS], int numArgs)
{
	int op = identify_cmd(args[0]);
	
	if((*args[numArgs] != '%') && (op != -1))
	{																		/*homemade function in fg*/
		job_list.job_insert(1, FG, args[0]);
		run_command(op,args,numArgs);
		//TODO consider array update (add "FINISH" state)
	}
	else
	{	
		char* argv[MAX_ARGS];
		// Arrange the args in argv[] for execv()
		for (int i = 0; i < numArgs + 1; i++)
			strcpy(argv[i], args[i+1]);
		argv[numArgs + 1] = NULL;
																
		pid_t pid = fork();		
		if(pid < 0)
		{
			perror("fork fail");
			exit(1);
			//TODO delete job
		}
		else if(pid == 0)																/*child code*/
		{
			if((op != -1)){									/*external command*/
				setpgrp();
				execvp(args[0], argv);
				//execvp never returns unless upon error
				perror("execvp fail");
				exit(1); 
			}
			else run_command(op,argv,numArgs);				/*homemade command*/
		}
		else{ 																			/*father code*/
			if((op != -1) && (*args[numArgs] != '%'))		/*wait for external command in fg*/
			{
				job_list.job_insert(pid, FG, args[0]);
				int exit_state;
				waitpid(pid, &exit_state, 0); 	
				if(WIFEXITED(exit_state)) //determines if a child exited with exit()
				{
				if(!WEXITSTATUS(exit_state))
					cout << "error: external command in fg had exit status != 0\n";
				else
					cout << "external command in fg finished successfully\n";
				}
			}
			else{											//father of external/homemade command in BG
				//consider masking method
				job_list.job_insert(pid, BG, args[0]);
			}
			
			
		}
	}

	return 0;
}

// @brief identifys which command and returns the corresponding index
// @param gets a string containing a command
int identify_cmd(char* cmd){
	for (int i=0; i<9; i++){
		if(!strcmp(commands[i], cmd)){
			return i ;
		}
	}
	return -1 ;
}

// @brief runs the command identified
int run_command(int op, char* args[MAX_ARGS], int numArgs){
	switch (op){
		case 0	: showpid(numArgs); 		break;
		case 1	: pwd(numArgs); 			break;
		// case 2	: cd(arg[1]); 			break;
		// case 3	: jobs(); 				break;
		// case 4	: kill(arg[1], arg[2]); break;
		// case 5	: fg(arg[1]); 			break;
		// case 6	: bg(arg[1]); 			break;
		case 7	: quit(/*args[1],*/ numArgs); 				break;
		// case 8	: diff(arg[1],arg[2]); 	break;
	}
}

int showpid (int numArgs){
	func_HandleConfigPack();
	if(numArgs != 0){
		cout << "error: showpid: expected 0 arguments\n";
		return 1 ;
	}
	int pid  = getpid();
	cout << "pid is " << pid << "\n" ;
	return 0 ;
}
int pwd(int numArgs){
	func_HandleConfigPack();
	if(numArgs != 0){
		cout <<"error: pwd: expected 0 arguments\n";
		return 1;
	}
	char buffer[MAX_LINE_SIZE]="/n";
	getcwd(buffer,MAX_LINE_SIZE);
	if(buffer==nullptr){
		cout <<"getcwd failed\n";
		return 1;
	}
	cout << buffer << endl;
	return 0;
}
// int cd(int numArgs,char* path [MAX_LINE_SIZE]){
// 	func_HandleConfigPack();
// 	if (numArgs!=1){
// 		cout <<  "error: cd: expected 1 arguments";
// 		return 1;
// 	}
// 	if(!strcmp(path,"-")){
// 		if()
// 	}
// }
int quit(/*char* args[1],*/ int numArgs){
	//TODO think about the case of running in bg

	// if((numArgs == 1) & (args[1] != "kill")){
	// 	cout << "error: quit: only kill command is possible\n";
	// 	return 1;
	// }
	if(numArgs > 1){
		cout <<"error: quit: too many arguments\n";
		return 1;
	}
	// if((numArgs == 1) & (args[1] == "kill")){
	// 	//TODO kill all jobs
	// }
	cout << "innitalizing shot down\n" ;
	return 0;
}


/*=============================================================================
* Function handlers
=============================================================================*/
//handler for ctrl+c
void func_handleFGKill(int sig) {
	// sigset_t maskSet;
	// sigemptyset(&maskSet);            // Start with an empty set
	// sigaddset(&maskSet, SIGTERM);
	sigset_t maskSet, oldSet;
	sigfillset(&maskSet);
	sigprocmask(SIG_SETMASK, &maskSet, &oldSet);

	job_list.job_FG_remove();
	cout << "process " << job_list.get_FG_pid() << " was killed\n";

	sigprocmask(SIG_SETMASK, &oldSet, &maskSet);
	return;
}

//handler for ctrl+z
void func_handleFg2Bg(int sig) {
	sigset_t maskSet, oldSet;
	sigfillset(&maskSet);
	sigprocmask(SIG_SETMASK, &maskSet, &oldSet);

	pid_t pid = fork();
	if(pid < 0)
	{
		perror("fork fail");
		sigprocmask(SIG_SETMASK, &oldSet, &maskSet);
		exit(1);
		//TODO in a perfect world we would also delete the job
	}
	else if(pid == 0){ 
		cout << "function got ctrl+z to the face\n";
		raise(SIGSTOP);			//child stops itself
	}
	else
	{
		/*DT maintaince by father*/
		job_list.job_insert(pid, STOPPED, job_list.get_FG_command());
		job_list.job_FG_remove();

		cout << "process " << job_list.get_FG_pid() << " was stopped\n";
		sigprocmask(SIG_SETMASK, &oldSet, &maskSet);
		return;
	}
}

//@brief: Handlers configuration pack	(TO PUT IN ALL FUNCTIONS)
void func_HandleConfigPack(){
	struct sigaction sb = { sb.sa_handler = &func_handleFGKill };
	sb.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sb, nullptr);

	struct sigaction sc = { sc.sa_handler = &func_handleFg2Bg };
	sc.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &sc, nullptr);
}


	
