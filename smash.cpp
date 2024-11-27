//smash.c

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>
#include "commands.h"
#include "signals.h"
#include <iostream>
#include <time.h>

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

class job {
	public:
	int pid ;
	string command;
	time_t time_stamp;
	char status ;
	bool full;
};

class job_arr {
	job jobs[MAX_ARGS+1] ;
	int job_count ;
	int free_idx ;

	public:
	job_arr(){
		job_count = 0;
		free_idx = 1;
	}

	int job_insert(int pid, int status, string command){
		if(job_count >= 100){
			cout << "fail to insert job, maximum jobs\n";
		}
		jobs[free_idx].pid = pid;
		jobs[free_idx].status = status;
		jobs[free_idx].command = command;
		jobs[free_idx].time_stamp = time(NULL);
		job_count++;
		for(int i=1; i<MAX_ARGS+1; i++){
			if(!jobs[i].full){
				free_idx = i;
				break;
			}
		}
	}

	int job_remove(int pid){
		for(int i=1; i<MAX_ARGS+1; i++){
			if(jobs[i].pid == pid){
				free_idx = (free_idx<i) ? free_idx : i;
				job_count--;
				jobs[i].full = 0;
				return 0;
			}
		}
		cout << "fail to remove job\n";
		return 1;
	}
	int stat_change(int pid, char stat){
		for(int i=1; i<MAX_ARGS+1; i++){
			if(jobs[i].pid == pid){
				jobs[i].status=stat;
				return 0;
			}
		}
		cout << "fail to change status\n";
		return 1;
	}

	void print(){
		for(int i=1; i<MAX_ARGS+1; i++){
			cout << i <<" "<< jobs[i].command << " " << jobs[i].pid << " " << difftime(time(NULL), jobs[i].time_stamp);
			if(jobs[i].status==STOPPED){
				cout << "(stopped)";
			}
			cout << endl;
		}
	}
	int job_2_front(int pid){
		for(int i=1; i<MAX_ARGS+1; i++){
			if(jobs[i].pid==pid){
				jobs[0].pid = pid;
				jobs[0].command = jobs[i].command ;
				jobs[0].time_stamp = jobs[i].time_stamp;
				jobs[0].status = jobs[i].status;
				jobs[0].full = jobs[i].full;
				jobs[i].full=0;
				free_idx = (free_idx<i) ? free_idx : i;
				job_count--;
				return 0;
			}
		}
		cout << "job_2_front failed\n";
		return 1;
	}
};

/*=============================================================================
* global variables & data structures
=============================================================================*/
char _line[MAX_LINE_SIZE];



/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char* argv[])
{
	job_arr jobs_arr ;
	jobs_arr.free_idx = 1 ;
	jobs_arr.job_ctr = 0 ;
	for(int i; i<MAX_ARGS+1; i++)
		jobs_arr.jobs[i] = NULL ;

	char _cmd[MAX_LINE_SIZE];
	while(1)
	{
		printf("smash > ");
		fgets(_line, MAX_LINE_SIZE, stdin);
		strcpy(_cmd, _line);
		_cmd[strlen(_line) + 1] = '\0';
		//execute command

		//initialize buffers for next command
		_line[0] = '\0';
		_cmd[0] = '\0';
	}

	return 0;
}
