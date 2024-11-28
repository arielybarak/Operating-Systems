#include <stdlib.h>
#include "commands.h"
#include "signals.h"
#include <iostream>
#include <time.h>
#include <unistd.h>

using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::prev;

#define FG   '1'
#define BG 	 '2'
#define STOPPED '3'

class job {
	public:
	pid_t pid ;
	char* command;
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
	int fg_job_insert(char* command){
		jobs[0].command = command;
		jobs[0].time_stamp = time(NULL);
	}

	int bg_job_insert(int pid, int status, char* command){
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