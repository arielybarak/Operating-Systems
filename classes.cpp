/*=============================================================================
* includes 
=============================================================================*/
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "commands.h"
#include "signals.h"
#include "classes.h"
#include <sys/wait.h>

/*=============================================================================
* using class
=============================================================================*/
using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::prev;

/*=============================================================================
* defines class
=============================================================================*/
#define FG   '1'
#define BG 	 '2'
#define STOPPED '3'

extern job_arr job_list;
extern pid_t complex_pid;
extern pid_t complex_state;
extern int complex_i;

/*=============================================================================
* job class functions
=============================================================================*/

job::job(){
	pid=0;
	*command='\0';
	time_stamp=0;
	status=0;
	full=0;
	*prev_wd='\0';
}
/*=============================================================================
* job_arr class functions
=============================================================================*/
// @brief job arr constructor
job_arr :: job_arr(){
	job_count = 0;
	free_idx = 1;
}
/*	@brief returns the index that contains the process with pid as its pid, 
	returns -1 if process with pid wasnt found
  	@param pid of wanted process*/
int job_arr::get_job_idx(pid_t pid){
	for(int i=0; i<=MAX_ARGS+1;i++){
		if((jobs[i].pid==pid)&&(jobs[i].full)){
			return i;
		}
	}
	return -1;
}

int job_arr::job_insert(pid_t pid, char status, char* command, bool is_external, bool complex_op){
	if(job_count >= 100){
		cout << "fail to insert job, job list is full\n";
		return 1;
	}
	// cout<<"job_insert\n";
	if(status == FG){
		strcpy(jobs[0].command, command);
		jobs[0].pid=pid;
		jobs[0].full = true;
		jobs[0].is_external = is_external;
		//cout << " in slot 0\n";
		return 0;
	}
	if(complex_op)
		complex_i = free_idx;
	
	jobs[free_idx].full = true;
	jobs[free_idx].pid = pid;
	jobs[free_idx].status = status;
	jobs[free_idx].is_external = is_external;
	strcpy(jobs[free_idx].command, command);
	jobs[free_idx].time_stamp = time(NULL);
	job_count++;
	for(int i=free_idx; i<MAX_ARGS+1; i++){
		if(!jobs[i].full){
			free_idx = i;
			break;
		}
	}
	return 0;
}

void job_arr::fg_job_remove(pid_t pid, char status){

	if(WIFSTOPPED(status)){									//fg stopped
		jobs[0].status = STOPPED;	
		job_insert(pid, STOPPED, jobs[0].command, true, 0);
	}
	else if(pid == jobs[0].pid)							//fg reaped
		jobs[0].full = false;		
	
	
	return;
}


void job_arr::job_remove(){
	int status;	
	pid_t pid;

	for(int i=1; i<MAX_ARGS+1; i++){							/*processes in BG*/
		if(jobs[i].full == true){

			pid = waitpid(jobs[i].pid, &status, WNOHANG | WUNTRACED | WCONTINUED); 
			if (pid == -1){ 
				std::perror("smash error: waitpid failed");
				return;
			}
			// if(pid == complex_pid)
			// 	complex_state = status;
				
			// if(WIFSTOPPED(status))									//proc stopped
			// 	jobs[i].status = STOPPED;
			
			// else if(WIFCONTINUED(status))							//proc continued
			// 	jobs[i].status = BG;
				
			else if(pid == jobs[i].pid){							//proc reaped
				free_idx = (free_idx<i) ? free_idx : i;
				job_count--;
				jobs[i].full = false;
			}
		}
	}
	return;
}

//  complex_pid;
// extern pid_t complex_state;
// extern int complex_i;



void job_arr::complexJob_remove(){
	pid_t pid;

	pid = waitpid(complex_pid, &complex_state, WUNTRACED | WCONTINUED);

	if (pid == -1){ 
		std::perror("smash error: waitpid failed");
		complex_state = 1;
		return;
	}
		
	// if(WIFSTOPPED(complex_state))									//proc stopped
	// 	jobs[complex_i].status = STOPPED;
	
	// else if(WIFCONTINUED(complex_state))							//proc continued
	// 	jobs[complex_i].status = BG;
		
	else if(pid == complex_pid){									//proc reaped
		free_idx = (free_idx < complex_i) ? free_idx : complex_i;
		job_count--;
		jobs[complex_i].full = false;
	}

	return ;
}



int job_arr::stat_change(int pid, char stat){
	for(int i=1; i<MAX_ARGS+1; i++){
		if(jobs[i].pid == pid){
			jobs[i].status=stat;
			return 0;
		}
	}
	cout << "fail to change status\n";
	return 1;
}
void job_arr::print(){
	for(int i=1; i<MAX_ARGS+1; i++){
		if(jobs[i].full){
			cout << "[" << i << "] "<< jobs[i].command << ": " << jobs[i].pid <<
			" " << difftime(time(NULL), jobs[i].time_stamp) << " secs";
			if(jobs[i].status==STOPPED){
				cout << " (stopped)";
			}
			cout << endl;
		}
	}
}
void job_arr::print_fg_job(){
		cout << "print_fg_job: " << jobs[0].command << " " << jobs[0].pid;
		cout << endl;
}
int job_arr::job_2_front(int pid){
	for(int i=1; i<MAX_ARGS+1; i++){
		if(jobs[i].pid==pid){
			jobs[0].pid = pid;
			strcpy(jobs[0].command,jobs[i].command);
			jobs[0].time_stamp = jobs[i].time_stamp;
			jobs[0].status = FG;
			jobs[0].full = true;
			jobs[i].full=false;
			free_idx = (free_idx<i) ? free_idx : i;
			job_count--;
			return 0;
		}
	}
	cout << "job_2_front failed\n";
	return 1;
}