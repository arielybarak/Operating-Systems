#ifndef classes_H
#define classes_H

#define MAX_CMD_LEN 10

#define FG   '1'
#define BG 	 '2'
#define STOPPED '3'
#define MAX_LINE_SIZE 80
#define MAX_ARGS 100


class job_arr {

    class job {
		public:
		pid_t pid ;
		char command[MAX_CMD_LEN];
		time_t time_stamp;
		char status ;
		bool full;
		char* prev_wd[MAX_LINE_SIZE];

		job(){
			pid=NULL;
			*command='\n';
			time_stamp=0;
			status=0;
			full=false;
			*prev_wd="\0";
		}
	}; 
	job jobs[MAX_ARGS+1]; 
	int job_count;
	int free_idx;

public:

    job_arr();
    // int fg_job_insert(char* command);
	pid_t get_FG_pid();
	char* get_FG_command();
	int job_FG_remove();
    int job_insert(int pid, int status, char* command);		//united with fg_job_insert by Barak
    int job_remove(int pid);
    int stat_change(int pid, char stat);
    void print();
	void print_fg_job();
    int job_2_front(int pid);
};

// job_arr job_list;
#endif