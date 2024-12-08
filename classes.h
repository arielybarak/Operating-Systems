#ifndef classes_H
#define classes_H

#define MAX_CMD_LEN 10

#define FG   '1'
#define BG 	 '2'
#define STOPPED '3'
#define MAX_LINE_SIZE 80
#define MAX_ARGS 100



/*=============================================================================
* job class
=============================================================================*/

class job {
	public:
	pid_t pid ;
	char command[MAX_CMD_LEN];
	time_t time_stamp;
	char status;
	bool full;
	char prev_wd[MAX_LINE_SIZE];
	bool is_external;

	job();
}; 

/*=============================================================================
* job_arr class
=============================================================================*/


class job_arr {
 
	int job_count;
	int free_idx;

public:
	job jobs[MAX_ARGS+1];

    job_arr();
	int get_job_idx(pid_t pid);
    int stat_change(int pid, char stat);
    void print();
	void print_fg_job();
	//---------list handeling---------------------------
	int job_insert(pid_t pid, int status, char* command);
	void fg_job_remove();
    int job_remove(int pid);
	int job_2_front(int pid);
	// -------------------------------------------------
};
#endif