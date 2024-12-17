//commands.cpp
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <vector>
#include <errno.h>

using namespace std;

#include "signals.h"
#include "commands.h"
#include "classes.h"
using namespace std;

#define FG  '1'
#define BG 	'2'
#define STOPPED '3'
#define FAIL '4'

#define SHOWPID 0
#define PWD 	1
#define CD 		2
#define JOBS 	3
#define KILL 	4
#define FG_F 	5
#define BG_F	6
#define QUIT 	7
#define DIFF 	8


const char* commands[9] = {"showpid","pwd","cd","jobs","kill","fg","bg",
															"quit","diff"};
extern job_arr job_list;
extern vector<vector<char*>> command_vec;
extern vector<bool> token;	
pid_t complex_pid = -1;
pid_t complex_state = -1;
int  complex_i;
bool flag;


/**
* @brief Runs the command with the corresponding op code
* @param op op code of desired function
* @param args the arguments for the function
* @param numArgs number of arguments in args
* @return 0 if the function finished successfuly, 1 otherwise.
*/
int run_command(int op, char* args[MAX_ARGS], int numArgs);

/**
* @brief identifies the which command was run
* @param cmd the command in string form
* @return op code of the command to run or -1 if its external
*/
int identify_cmd(char* cmd);


void Complex_SpaceCheck(char* &tmp, int &numArgs)
{
	char* tmp2 = new char[MAX_ARGS];
	int i=0, j=0;
	bool flag = false;
	
	if((!strcmp(tmp, "&&")) || (!strcmp(tmp, ";"))){
		if(!strcmp(tmp, "&&")) 
			token.push_back(true);
		else
			token.push_back(false);

		command_vec.emplace_back();
		return;
	} 

	while (tmp[i] != '\0') {

		if((tmp[i] == '%') && (tmp[i+1] != '\0')){

			if(i>0){
				char* tmp2_copy = new char[j+1];
				strncpy(tmp2_copy, tmp2, j);
				tmp2_copy[j] = '\0';
				command_vec.back().push_back(tmp2_copy);
				j = 0;
			}
			char* tmp2_copy = new char[2];
			strcpy(tmp2_copy,"%");
			tmp2_copy[1] = '\0';
			command_vec.back().push_back(tmp2_copy);
			flag = true;
			i++;
		}

		if ((tmp[i] == ';') || ((tmp[i] == '&') && (tmp[i+1] == '&'))) {
			
			tmp2[j] = '\0'; 
			j = 0; 
			if(!(tmp[i] == ';'))
				i++;

			if(((i>0) && (tmp[i] == ';')) || ((i>1) && !(tmp[i] == ';'))){

				if(!flag){
					char* tmp2_copy = new char[strlen(tmp2) + 1];
					strcpy(tmp2_copy, tmp2);
					command_vec.back().push_back(tmp2_copy);
				}
			}

			token.push_back(!(tmp[i] == ';'));
			command_vec.emplace_back();
		}
		else 
			tmp2[j++] = tmp[i]; 
		
		i++;
	}

	tmp2[j] = '\0';
	if(strlen(tmp2)>0){
		char* tmp2_copy = new char[strlen(tmp2) + 1];
        strcpy(tmp2_copy, tmp2);
		command_vec.back().push_back(tmp2_copy);
	}
	delete[] tmp2;
}


int parseCommand(char* line)
{
	char delimiters[]=" \t\n"; 
	char *tmp;
	int numArgs = 0;
	
	tmp = strtok(line, delimiters); 				
	if(!tmp)
		return INVALID_COMMAND; 					//no tokens were found
	if (!command_vec.empty())
		cout << "smash error: complex_vec isn't empty\n";

    command_vec.emplace_back();
	Complex_SpaceCheck(tmp, numArgs);

	while((numArgs < MAX_ARGS) && (tmp))
	{
		numArgs++;
		tmp = strtok(NULL, delimiters); 
		if(tmp == NULL){
			numArgs--;
			break;
		}
		Complex_SpaceCheck(tmp, numArgs);
	}
	token.push_back(false);
	
	if (command_vec.empty())
    	cout << "Error: command_vec or token is empty at function start.\n";
	return numArgs;
}




void command_manager(int numArgs, char* command)
{
	char* args[MAX_ARGS]; 
	char status;

	while(!command_vec.empty()){

		complex_state = -1;
		complex_pid = -1;
		
		job_list.job_remove();						/*lets clean bg world*/
		copy(command_vec.front().begin(), command_vec.front().end(), args);

		strcpy(command, args[0]); 
		for (size_t i = 1; i < command_vec.front().size(); ++i) {
			strcat(command, " ");   
			strcat(command, args[i]); 
		}

		status = processReturnValue(args, command_vec.front().size() - 1, command, token.front());


		if(status == FAIL)
			cout<<"status command failed\n";
		
		if(token.front()){								/*complex type "&&"*/ 

			if(status == BG)
				job_list.complexJob_remove();		

			if(complex_state > 0){
				complex_state = -1;
				complex_pid = -1;
				command_vec.clear();
				token.clear();
				return;
			}
		}
		command_vec.erase(command_vec.begin());
		token.erase(token.begin());
	}
}


/**
* @brief brief process the given command and run it
* @param args the arguments for the function
* @param numArgs number of arguments in args
* @param command command to run in string form
* @param complex_op flag if the command is complex or not
* @return 0 if the command finished successfuly, 1 otherwise.
*/
char processReturnValue(char* args[MAX_ARGS], int numArgs, char* command, bool complex_op)
{
	pid_t pid;
	char status = FG;
	int ret;
	args[numArgs+1] = NULL;
	
	status = (!strcmp(args[numArgs], "%")) ? BG : FG;
	int last_arg_len=strlen(args[numArgs]);
	if((status==FG)&&(args[numArgs][last_arg_len-1]=='%')){
		status=BG;
		args[numArgs][last_arg_len-1]='\0';
		numArgs++;
	}

	int op = identify_cmd(args[0]);
	if((status == BG) & (op == 5)){
		cout << "smash error: fg: cannot run in background" << endl;
		return 1;/////might cause problems
	}

	if(((status == FG) && (op > -1)) || (op == 5)){								/*homemade function in fg*/
		//cout<<"homemade function in fg\n";
		pid = getpid();
		job_list.job_insert(pid,FG,command,false,complex_op);
		ret = run_command(op,args,numArgs);								
		job_list.fg_job_remove(pid, 0);
		if(complex_op)
			complex_state = ret;
	}
	else
	{
		if(status == BG)
			numArgs--;
		
		args[numArgs+1] = NULL;	//for execvp

		pid = fork();	

		if(complex_op)	
			complex_pid = pid;

		if(pid < 0)
		{
			perror("fork fail");
			exit(1);
			//TODO delete job
		}
		else if(pid==0)										/*child code*/
		{
			if(op == -1){									/*external command*/
				// cout << "(son): external\n";
				setpgrp();
				execvp(args[0], args);
				if(errno==ENOENT){
					cout<<"smash error: external: cannot find program"<<endl;
				}
				else{
					cout<<"smash error: external: invalid command"<<endl;
				}
				//execvp never returns unless upon error
				exit(1); 
			}
			else{
				ret = run_command(op,args,numArgs);				/*homemade command*/
				exit(ret);
			}
		}
		else{ 																			/*father code*/
			job_list.job_insert(pid, status, command, (op == -1), complex_op);
			if((op == -1) && (status == FG))						/*father wait for external command in fg*/
			{
				//cout <<"FG external wait\n";
				int exit_state;
				if (waitpid(pid, &exit_state, WUNTRACED) == -1)	
				{
					std::perror("smash error: waitpid failed");
					return FAIL;
				}	
				if(complex_op)
					complex_state = exit_state;
				job_list.fg_job_remove(pid, exit_state);

				if(WIFEXITED(exit_state) && (WEXITSTATUS(exit_state)!=0)) //determines if a child exited with exit()
				return status;
			}
		}
	}
	return status;
}


/**
* @brief Runs the command with the corresponding op code
* @param op op code of desired function
* @param args the arguments for the function
* @param numArgs number of arguments in args
* @return 0 if the function finished successfuly, 1 otherwise.
*/
int identify_cmd(char* cmd){
	for (int i=0; i<9; i++){
		if(!strcmp(commands[i], cmd)){
			return i ;
		}
	}
	return -1 ;
}

/**
* @brief prints smash PID
*/
void showpid (){
	cout << "smash pid is " << job_list.jobs[0].pid << "\n" ;
	return;
}

/**
* @brief Prints current working directory
* @return 0 if the function finished successfuly, 1 otherwise.
*/
int pwd(){
	char buffer[MAX_LINE_SIZE]="\0";
	char* retval=getcwd(buffer,MAX_LINE_SIZE);
	if(retval==NULL){
		cout <<"getcwd failed\n";
		return 1;
	}
	cout << buffer << endl;
	return 0;
}

/**
* @brief Changes current working directory
* @param path string containing the desired path
* @return 0 if the function finished successfuly, 1 otherwise.
*/
int cd(char* path){
	int retval=0;
	bool prev_jump=false;
	char temp [MAX_LINE_SIZE];
	if(!strcmp(path,"-")){
		if(job_list.jobs[0].prev_wd[0]=='\0'){
			cout << "smash error: cd: old pwd not set" << endl;
			return 1;
		}
		prev_jump=true;
		strcpy(temp,job_list.jobs[0].prev_wd);
		getcwd(job_list.jobs[0].prev_wd,MAX_LINE_SIZE);
		retval = chdir(temp);
	}
	if(!prev_jump){
		getcwd(temp,MAX_LINE_SIZE);
		retval = chdir(path);
		if(retval){ // @todo check the error(noam)
			if(errno==ENOENT){
				cout << "smash error: cd: target directory does not exist" << endl;
				return 1;
			}
			else{
				std::perror("smash error: chdir failed");
				return 1;
			}
		}
		else{
			strcpy(job_list.jobs[0].prev_wd,temp);
		}
	}
	return 0;
}

/**
* @brief prints all jobs in the list
* @return 0 always
*/
int jobs(){
	job_list.print();
	return 0;
}

/**
* @brief sends signal to desired process
* @param signum signal number to send
* @param job_id job number to send the signal to
* @return 0 if the signal was sent, 1 otherwise
*/
int kill_func(int signum , int job_id){
	if(!job_list.jobs[job_id].full){
		cout << "job id " << job_id << " does not exist"<< endl;
		return 1;
	}
	int retval=kill(job_list.jobs[job_id].pid,signum);
	if(retval!=0){
		perror("smash error: kill failed");
	}
	else{
	cout << "signal " << signum << " was sent to pid " << 
									job_list.jobs[job_id].pid << endl;
	}
	return retval;
}

/**
* @brief moves job from list to the foreground
* @param job_id_str job id in the form of a string
* @param numArgs number of arguments added to the command
* @return 0 if the job was successfuly moved, 1 otherwise
*/
int fg(char* job_id_str, int numArgs){
	int job_id;
	if(numArgs==0){
		job_id=0;
		for(int i=0;i<MAX_ARGS+1;i++){
			if(job_list.jobs[i].full){
				job_id=i;
			}
		}
		if(job_id==0){
			cout << "smash error: fg: jobs list is empty"<<endl;
			return 1;
		}
	}
	else{
		char* endptr;
		job_id = strtol(job_id_str, &endptr, 10);
		if (*endptr != '\0' || job_id<1||100<job_id){
			cout << "smash error: fg: invalid arguments" <<endl;
			return 1;
		}
		if(!job_list.jobs[job_id].full){
			cout <<"smash error: fg: job id "<< job_id << " does not exist" <<endl;
			return 1;
		}
    }
	pid_t job_pid=job_list.jobs[job_id].pid;
	int front_mov=job_list.job_2_front(job_pid);
	job_list.jobs[0].is_external=true;
	if(front_mov){
		cout << "move to front failed"<<endl; 
		return 1;
	}
	cout << job_list.jobs[0].command<< ": " << job_list.jobs[0].pid << endl;
	int status;
	int retval=kill(job_pid,SIGCONT);
	if(retval!=0){
		perror("smash error: kill failed");
	}
    if (waitpid(job_pid, &status, WUNTRACED) == -1) {
        perror("smash error: waitpid failed");
        return 1;
    }
	if(WIFSTOPPED(status)!=0){
		job_list.fg_job_remove(job_pid,(char)status);
	}
	return 0;
}

/**
* @brief continues a process stopped in the background
* @param job_id_str job id in the form of a string
* @param numArgs number of arguments added to the command
* @return 0 if the job was continued, 1 otherwise
*/
int bg(char* job_id_str, int numArgs){
	int job_id;
	if(numArgs==0){
		job_id=0;
		for(int i=0;i<MAX_ARGS+1;i++){
			if(job_list.jobs[i].full&&job_list.jobs[i].status==STOPPED){
				job_id=i;
			}
		}
		if(job_id==0){
			cout << "smash error: bg: there are no stopped jobs to resume" <<endl;
			return 1;
		}
	}
	else{
		char* endptr;
		job_id = strtol(job_id_str, &endptr, 10);
		if (*endptr != '\0' || job_id<1||100<job_id){
			cout << "smash error: fg: invalid arguments" <<endl;
			return 1;
		}
		if(!job_list.jobs[job_id].full){
			cout <<"smash error: bg: job id "<< job_id << " does not exist" <<endl;
			return 1;
		}
		if(job_list.jobs[job_id].status==BG){
			cout <<"smash error: bg: job id "<< job_id << " is already in background" << endl;
			return 1;
		}
    }
	cout << job_list.jobs[job_id].command<< ": " << job_list.jobs[job_id].pid << endl;
	int ret_val=kill(job_list.jobs[job_id].pid,SIGCONT);
	if(ret_val!=0){
		perror("smash error: kill failed");
		return 1;
	}
	else{
		job_list.stat_change(job_list.jobs[job_id].pid,BG);
		return 0;
	}
}

/**
* @brief exits smash, kills all jobs if asked to
* @param numArgs number of arguments added to the command
* @param arg_1 contains the word "kill" to indicate
* @return doesnt return if succesful, returns 1 otherwise
*/
int quit(int numArgs, char* arg_1) {
    if (numArgs == 1) {
		if(!strcmp(arg_1, "kill")){
			for (int i = 1; i < MAX_ARGS + 1; i++) {
				if (job_list.jobs[i].full) {
					bool terminated = false;
					cout << "[" << i << "] " << job_list.jobs[i].command << " - ";

					// Attempt to continue the process
					if (kill(job_list.jobs[i].pid, SIGCONT) == -1) {
						perror("smash error: kill failed");
						return 1;
					}

					// Send SIGTERM
					if (kill(job_list.jobs[i].pid, SIGTERM) == -1) {
						perror("smash error: kill failed");
						return 1;
					}
					cout << "sending SIGTERM... ";
					// Wait for the process to terminate
					for (int j = 0; j < 5; j++) {
						int status;
						pid_t result = waitpid(job_list.jobs[i].pid, &status, WNOHANG);

						if (result == -1) {
							perror("smash error: waitpid failed");
							return 1; // Stop waiting if there's an error
						}

						if (result > 0) { // Process has terminated
							if (WIFEXITED(status) || WIFSIGNALED(status)) {
								cout << "done" << endl;
								terminated = true;
								break;
							}
						}

						sleep(1); // Wait 1 second before checking again
					}

					// If not terminated, send SIGKILL
					if (!terminated) {
						cout << "sending SIGKILL... done" << endl;
						if (kill(job_list.jobs[i].pid, SIGKILL) == -1) {
							perror("smash error: kill failed");
							return 1;
						}
					}

					// Clean up the job entry
					job_list.jobs[i].full = false;
				}
			}
			exit(0);
		}
		else{
			cout<< "smash error: quit: unexpected arguments"<<endl;
			return 1;
		}
    }
	else{
		exit(0);
	}
    // Clean up and exit
}

/**
* @brief compares two files passed to it prints 0 if contents match and 1 otherwise
* @param path1 path to first file
* @param path2 path to second file
* @return returns 0 if finished succesfuly and 1 otherwise
*/
int diff(char* path1, char* path2) {
    int f1, f2;
    int ret_val1, ret_val2;
    char c1, c2;
    struct stat stat_buf;

    // Check if path1 is a valid file
    if (stat(path1, &stat_buf) == -1) {
        if (errno == ENOENT) {
            cout << "smash error: diff: expected valid paths for files" << endl;
        } 
		else {
            perror("smash error: stat failed");
        }
        return 1;
    }
    if (!S_ISREG(stat_buf.st_mode)) {
        cout << "smash error: paths are not files" << endl;
        return 1;
    }

    // Check if path2 is a valid file
    if (stat(path2, &stat_buf) == -1) {
        if (errno == ENOENT) {
            cout << "smash error: diff: expected valid paths for files" << endl;
        } else {
            perror("smash error: stat failed");
        }
        return 1;
    }
    if (!S_ISREG(stat_buf.st_mode)) {
        cout << "smash error: diff: paths are not files" << endl;
        return 1;
    }

    // Open files
    f1 = open(path1, O_RDONLY);
    if (f1 == -1) {
        perror("smash error: open failed");
        return 1;
    }
    f2 = open(path2, O_RDONLY);
    if (f2 == -1) {
        perror("smash error: open failed");
        close(f1);
        return 1;
    }

    // Compare files character by character
    while (1) {
        ret_val1 = read(f1, &c1, 1); // Read one char from f1
        if (ret_val1 == -1) {
            perror("smash error: read failed");
            close(f1);
            close(f2);
            return 1;
        }
        ret_val2 = read(f2, &c2, 1); // Read one char from f2
        if (ret_val2 == -1) {
            perror("smash error: read failed");
            close(f1);
            close(f2);
            return 1;
        }

        if (c1 != c2) { // Found a char not matching
            cout << "1" << endl;
            close(f1);
            close(f2);
            return 0;
        }

        if ((ret_val1 == 0) || (ret_val2 == 0)) { // End of file(s)
            break;
        }
    }

    // Files match
    cout << "0" << endl;

    // Close files
    if (close(f1) == -1) {
        perror("smash error: close failed");
        close(f2);
        return 1;
    }
    if (close(f2) == -1) {
        perror("smash error: close failed");
        return 1;
    }

    return 0;
}


/**
* @brief Runs the command with the corresponding op code
* @param op op code of desired function
* @param args the arguments for the function
* @param numArgs number of arguments in args
* @return 0 if the function finished successfuly, 1 otherwise.
*/
int run_command(int op, char* args[MAX_ARGS], int numArgs){

	switch (op){
		case SHOWPID	: { //call showpid
			if(numArgs!=0){
				cout << "smash error: showpid: expected 0 arguments\n";
				return 1;
			}
			showpid();
			return 0;
		}
		case PWD		: { //call pwd
			if(numArgs != 0){
				cout <<"smash error: pwd: expected 0 arguments\n";
				return 1;
			}
			return pwd();
		}
		case CD			: { //call cd
			if(numArgs!=1){
				cout << "smash error: cd: expected 1 arguments"<<endl;
				return 1;
			}
			return cd(args[1]);
		}
		case JOBS		: {	//call jobs
			if(numArgs!=0){
				cout << "smash error: jobs: expected 0 arguments\n";
				return 1;
			}
			jobs();
			return 0; 				
		}
		case KILL		: { //call kill_func
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
		}
		case FG_F		: {	//call fg
			if(numArgs>1){
				cout << "smash error: fg: invalid arguments" <<endl;
				return 1;
			}
			return fg(args[1],numArgs);
		}
		case BG_F		: {	//call bg
			if(numArgs>1){
				cout << "smash error: bg: invalid arguments" <<endl;
				return 1;
			}
			int retval=bg(args[1],numArgs);
			return retval;
		}
		case QUIT		: {	//call quit
			if(1<numArgs){
				cout << "smash error: quit: unexpected arguments" <<endl;
				return 1;
			}
			return quit(numArgs,args[1]);
		}
		case DIFF		: {	//call diff
			if(numArgs!=2){
				cout << "smash error: diff: expected 2 arguments" << endl;
				return 1;
			}
			diff(args[1],args[2]);
		}
		default: return 0;
	}
}


	
