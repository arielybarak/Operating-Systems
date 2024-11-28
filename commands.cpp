//commands.c
#include <stdlib.h>
#include "commands.h"
#include "signals.h"
#include <iostream>
#include <time.h>
#include <classes.cpp>
#include <sys/types.h>

#include <sys/wait.h>
#include <classes.cpp>

//example function for parsing commands

int run_command(int op);
int table();
int showpid (int numArgs);


int parseCmdExample(char* line)
{
	char* delimiters = " \t\n"; //parsing should be done by spaces, tabs or newlines
	char* cmd = strtok(line, delimiters); //read strtok documentation - parses string by delimiters
	if(!cmd)
		return INVALID_COMMAND; //this means no tokens were found, most like since command is invalid
	
	// char* args[MAX_ARGS];
	int numArgs = 0;
	args[0] = cmd; //first token before spaces/tabs/newlines should be command name
	for(int i = 1; i < MAX_ARGS; i++)
	{
		args[i] = strtok(NULL, delimiters); //first arg NULL -> keep tokenizing from previous call
		if(!args[i])
			break;
		numArgs++;
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

int processReturnValueExample(char* args[MAX_ARGS], int numArgs, job_arr 		job_array)
{
	int op = table();
	if((args[numArgs] != "%") && (op != -1)){		/*run in fg*/
		run_command(op);
		job_array.fg_job_insert(args[0]) ;
	}
	else{											/*run in bg*/
		pid_t pid = fork();
		if(pid < 0)
		{
			perror("fork fail");
			exit(1);
		}
		else if(pid == 0) //child code
		{
			if(op == -1){
				//do some work here - for example, execute an external command
				char* cmd = "/bin/ls";
				char* args[] = {"ls", "-l", NULL};
				execvp(cmd, args);
				//execvp never returns unless upon error
				perror("execvp fail");
				exit(1); //set nonzero exit code for father to read
			}
			else {
				run_command(op);
				
			}
		}

		else //father code
		{
			int status; 
			job_array.bg_job_insert(pid, status, args 0)
			waitpid(pid, &status, 0); //wait for child to finish and read exit code into status
			if(WIFEXITED(status)) //WIFEXITED determines if a child exited with exit()
			{
				int exitStatus = WEXITSTATUS(status);
				if(!exitStatus)
				{
					//exit status != 0, handle error	
				}
				else
				{
					//exit status == 0, handle success
				}
			}
		}
	}

	return 0;
}


int showpid (int numArgs){
	if(numArgs != 0){
		cout << "error: showpid: expected 0 arguments\n";
		return 1 ;
	}
	int pid  = getpid();
	cout << "pid is " << pid << "\n" ;
	return 0 ;
}

int table(){
	for (int i=0; i<9;i++){
		if(strcmp(commands[i], args[0]))
			return i ;
	}
	return -1 ;
}

int run_command(int op){
	switch (op){
		case 1	: showpid();
		// case 2	: pwd();
		// case 3	: cd(arg[1]);
		// case 4	: jobs();
		// case 5	: kill(arg[1], arg[2]);
		// case 6	: fg(arg[1]);
		// case 7	: bg(arg[1]);
		// case 8	: quit();
		// case 9	: diff(arg[1],arg[2]);
	}
}



	
