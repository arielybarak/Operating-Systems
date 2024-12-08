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
			cout << "this is " << i <<endl;
			return i;
		}
	}
	return -1;
}

int job_arr::job_insert(pid_t pid, int status, char* command){
	if(job_count >= 100){
		cout << "fail to insert job, job list is full\n";
		return 1;
	}
	if(status == FG){
		strcpy(jobs[0].command, command);
		jobs[0].pid=getpid();
		jobs[0].full = true;
		return 0;
	}
	jobs[free_idx].pid = pid;
	jobs[free_idx].status = status;
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

void job_arr::fg_job_remove(){
	jobs[0].full= false;
	jobs[0].prev_wd[0]='\0';
	return;
}

int job_arr::job_remove(int pid){
	for(int i=1; i<MAX_ARGS+1; i++){
		if(jobs[i].pid == pid){
			free_idx = (free_idx<i) ? free_idx : i;
			job_count--;
			jobs[i].full = false;
			return 0;
		}
	}
	cout << "fail to remove job\n";
	return 1;
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