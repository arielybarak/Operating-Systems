//commands.cpp
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

#include "signals.h"
#include "commands.h"
#include "classes.h"

using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::prev;

#define FG   '1'
#define BG 	 '2'
#define STOPPED '3'

const char* commands[9] = {"showpid","pwd","cd","jobs","kill","fg","bg",
															"quit","diff"};
extern job_arr job_list;
//example function for parsing commands

int run_command(int op, char* args[MAX_ARGS], int numArgs);;
int identify_cmd(char* cmd);

int parseCommand(char* line, char* args[])
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
	char status;
	status = (*args[numArgs] == '%') ? BG : FG;
	if((status == FG) && (op != -1))
	{												/*homemade function in fg*/
		// cout << "test: here is homemade in FG\n";
		job_list.job_insert(-1, FG, args[0],false);
		run_command(op,args,numArgs);
	}
	else
	{	
		if(*args[numArgs] == '%')
		/* @todo need to add exception for trying to run fg in bg using '%', fg cant run in bg.
		added the relevant error line fo that:
		cout << "smash error: fg: cannot run in background" << endl;*/
			numArgs--;
		//no use of argv
		args[numArgs+1] = NULL;	//for execvp
		// if((status == FG) && (op == -1)){
		// char temp[256];  // Ensure this buffer is large enough
		// 	strcpy(temp, "/usr/bin/");
		// 	strcat(temp, args[0]);
		// 	args[0] = strdup(temp);  // Duplicate and assign back to args[0]
		// 	cout << "args[0] is " << args[0] ;
		// }

		pid_t pid = fork();		
		if(pid < 0)
		{
			perror("fork fail");
			exit(1);
			//TODO delete job
		}
		else if(pid == 0)																/*child code*/
		{
			if(op == -1){									/*external command*/
				cout << "(son): here is external\n";
				setpgrp();
				execvp(args[0], args);
				//execvp never returns unless upon error
				perror("(son) execvp sadly fail");
				exit(1); 
			}
			else{
				run_command(op,args,numArgs);				/*homemade command*/
				cout << "(son): here is homemade in bg\n";
				// cout << "smash > ";
				exit(0);
			}
		}
		else{ 																			/*father code*/
			job_list.job_insert(pid, status, args[0], (op == -1));
			if((op == -1) && (status == FG))		/*father wait for external command in fg*/
			{
				cout << "(father): external in FG\n";
				int exit_state;
				if (waitpid(pid, &exit_state, WUNTRACED) == -1)			//error: doesnt return correctly (external in FG)
				// @todo add job_remove
				{
					std::perror("smash error: waitpid failed");
					return 1;
				}	
				cout << "exit_state: " << exit_state << endl;
				cout << "WEXITSTATUS(exit_state): " << WEXITSTATUS(exit_state) << endl;

				if(WIFEXITED(exit_state)) //determines if a child exited with exit()
				{
				if(WEXITSTATUS(exit_state)==0)
					cout << "error: external command in fg had exit status != 0\n";
				else
					cout << "external command in fg finished successfully\n";
				}
    
			}
			else{											//father of external/homemade command in BG
				// if(op == -1)
					// cout << "test(father): external in BG\n";
				// else cout << "test(father): homemade in BG\n";
				//consider masking method
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

void showpid (){
	int pid  = getpid();
	cout << "pid is " << pid << "\n" ;
	return;
}
int pwd(){
	char buffer[MAX_LINE_SIZE]="/n";
	getcwd(buffer,MAX_LINE_SIZE);
	if(buffer==nullptr){
		cout <<"getcwd failed\n";
		return 1;
	}
	cout << buffer << endl;
	return 0;
}
int cd(char* path){
	int retval=0;
	pid_t pid=getpid();
	int idx=job_list.get_job_idx(pid);
	bool prev_jump=false;
	if(idx==-1){
		cout << "job not found"<< endl;
		return 1;
	}
	char temp [MAX_LINE_SIZE];
	if(!strcmp(path,"-")){
		if(job_list.jobs[idx].prev_wd[0]=='\0'){
			cout << "error: cd: old pwd not set" << endl;
			return 0;
		}
		prev_jump=true;
		strcpy(temp,job_list.jobs[idx].prev_wd);
		getcwd(job_list.jobs[idx].prev_wd,MAX_LINE_SIZE);
		retval = chdir(temp);
	}
	if(!prev_jump){
		getcwd(temp,MAX_LINE_SIZE);
		retval = chdir(path);
		if(retval){
			cout << "error: cd: target directory does not exist" << endl;
		}
		else{
			strcpy(job_list.jobs[idx].prev_wd,temp);
		}
	}
	return retval;
}
void jobs(){
	job_list.print();
	return;
}
int kill_func(int signum , int job_id){
	if(!job_list.jobs[job_id].full){
		cout << "job id " << job_id << " does not exist"<< endl;
		return 1;
	}
	kill(job_list.jobs[job_id].pid,signum);
	cout << "signal " << signum << " was sent to pid " << 
									job_list.jobs[job_id].pid << endl;
	return 0;
}
// int fg(char* job_id_str, int numArgs){
// 	if(numArgs==1){
// 		char* endptr;
// 		int job_id = strtol(job_id_str, &endptr, 10);
// 		if (*endptr != '\0' || job_id<1||100<job_id){
// 			cout << "smash error: fg: invalid arguments" <<endl;
// 			return 1;
// 		}
// 		if(!job_list.jobs[job_id].full){
// 			cout << "smash error: fg: job id"<< job_id << "does not exist" <<endl;
// 			return 1;
// 		}
// 		pid_t job_pid=job_list.jobs[job_id].pid;
// 		job_list.job_2_front(job_pid);
// 		int status;
// 		pid_t pid = waitpid(pid,&status,NULL);

// 	}
// }
int quit(int numArgs, char* arg_1){

	if((numArgs == 1) && (!strcmp(arg_1,"kill"))){
	 	//TODO kill all jobs
		for(int i=1;i<MAX_ARGS+1;i++){
			if(job_list.jobs[i].full){
				bool terminated = false;
				cout << "["<<i<<"]" << job_list.jobs[i].command << " - " <<endl;
				kill(job_list.jobs[i].pid ,SIGTERM);
				cout << "sending SIGTERM... ";
				for (int i=0; i<5;i++){
					int status;
					pid_t result=waitpid(job_list.jobs[i].pid,&status,WNOHANG);
					if(result==-1){
						return-1;
					}
					if(WIFEXITED(status) || WIFSIGNALED(status)){
						cout<<"done"<<endl;
						terminated=true;
						break;
					}
					sleep(1);
				}
				if(!terminated){
					cout << "sending SIGKILL... done"<<endl;
					kill(job_list.jobs[i].pid, SIGKILL);
				}
			}
		}
	}
	exit(0);
}
int diff(char* path1, char* path2){
	
	int f1;
	int f2;
	int ret_val1;
	int ret_val2;
	char c1;
	char c2;

	f1 = (path1, O_RDONLY);
	if ( f1 == -1 ){
		perror("smash error: open failed");
		return 1;
	}
	f2 = open(path2, O_RDONLY);
	if ( f2 == -1 ){
		perror("smash error: open failed");
		if ( close(f1) == -1) {
			perror("smash error: close failed");
		}
		return 1;
	}
	while (1){
		ret_val1 = read(f1, &c1, 1); // read one char from f1
		if (ret_val1 == -1){
			perror("smash error: read failed");
			if ( close(f1) == -1) {
				perror("smash error: close failed");
			}
			if ( close(f2) == -1) {
				perror("smash error: close failed");
			}
			return 1;
		}
		ret_val2 = read(f2, &c2, 1); // read one char from f2

		if ( ret_val2 == -1 ){
			perror("smash error: read failed");
			if ( close(f1) == -1) {
				perror("smash error: close failed");
			}
			if ( close(f2) == -1) {
				perror("smash error: close failed");
			}
			return 1;
		}

		if( c1 != c2 ) { // found a char not matching
			break;
		}

		if ( (ret_val1 == 0) || (ret_val2 == 0) ){
			break;
		}
	} // end of while

	if ( c1 == c2 ){ // files match
		cout<<"0"<<endl;
	}

	else { // file don't match
		cout<<"1"<<endl;
	}

	// closing files
	if ( close(f1) == -1 ) {
		perror("smash error: close failed");
		if ( close(f2) == -1) {
			perror("smash error: close failed");
		}
		return 1;
	}
	if ( close(f2) == -1) {
		perror("smash error: fclose failed");
		return 1;
	}
	return 0;

}

// @brief runs the command identified
int run_command(int op, char* args[MAX_ARGS], int numArgs){
	switch (op){
		case 0	: {
			if(numArgs!=0){
				cout << "smash error: showpid: expected 0 arguments\n";
			}
			showpid();
			return 0;
			break;
		}
		case 1	: {
			if(numArgs != 0){
				cout <<"error: pwd: expected 0 arguments\n";
				return 1;
			}
			return pwd();
			break;
		}
		case 2	: { 
			if(numArgs!=1){
				cout << "smash error: cd: expected 1 arguments";
				return 1;
			}
			return cd(args[1]);
			break;
		}
		case 3	: {
			jobs();
			return 0; 				
			break;
		}
		case 4	: { 
    		if (numArgs != 2) {
        		cout << "smash error: kill: invalid arguments\n";
        		return 1;
    		}
    		char* endptr1;
    		char* endptr2;
    		// Convert the first argument (signal number) to an integer
    		int signum = strtol(args[1]+1, &endptr1, 10);
			// Convert the second argument (PID) to a process ID
			int job_id = strtol(args[2], &endptr2, 10);
			if (*endptr1 != '\0' || *endptr2 != '\0' || job_id<1||100<job_id||*args[1]!='-'||signum<1||signum>22) {
				cout << "smash error: kill: invalid arguments\n";
				return 1;
			}
			// If both arguments are valid, call kill_func
			return kill_func(signum, job_id);
			break;
		}
		// case 5	: {
		// 	if(numArgs>1){
		// 		cout << "smash error: fg: invalid arguments" <<endl;
		// 		return 1;
		// 	}
		// 	return fg(args[0],numArgs);
		// 	break;
		// }
		// // case 6	: bg(arg[1]); 			break;
		// case 7	: {
		// 	if(1<numArgs){
		// 		cout << "smash error: quit: unexpected arguments" <<endl;
		// 		return 1;
		// 	}
		// 	quit(numArgs,args[1]);
		// 	break;
		// }
		case 8	: {
			if(numArgs!=2){
				cout << "smash error: diff: invalid arguments" << endl;
				return 1;
			}
			diff(args[1],args[2]);
			break;
		}
	}
}

//cout << "pid: " << job_list.jobs[i].pid << "external: "<< job_list.jobs[i].is_external << " finnished\n";
int jobs_update(){
	int status;									
    pid_t pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED); 
	// if (pid == -1){ 
	// 		std::perror("smash error: waitpid failed");
	// 		return 1;
	// }
	if(WIFEXITED(status))							//WIFEXITED determines if a child exited with exit()
		cout << "child " << pid << " exited with exit()\n";
	

	int result = job_list.job_remove(pid, status);

	if(result==2)
		job_list.job_insert(pid, STOPPED, job_list.jobs[0].command, true);

	return result;
}




	
