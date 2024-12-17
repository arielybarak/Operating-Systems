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
	char command[MAX_LINE_SIZE];
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
	
	// Constructor for the job class, initializes all members to default values
    job_arr();
	/**
 	* @brief Returns the index that contains the process with the specified PID.
 	* @param pid The PID of the desired process.
 	* @return The index if found, or -1 if the process was not found.
 	*/
	int get_job_idx(pid_t pid);

	/**
 	* @brief Changes the status of a job.
 	* @param pid Process ID of the job.
 	* @param stat New status of the job.
 	* @return 0 on success, 1 if the job was not found.
 	*/
    int stat_change(int pid, char stat);

	/**
 	* @brief Prints all jobs in the job array.
 	*/
    void print();

	/**
	 * @brief Prints the foreground job.
 	*/
	void print_fg_job();
	//---------list handeling---------------------------

	/**
	* @brief Inserts a job into the job array.
	* @param pid Process ID of the job.
	* @param status Status of the job (e.g., FG, BG, STOPPED).
	* @param command Command string associated with the job.
	* @param is_external Boolean indicating if the job is external.
	* @param complex_op Boolean indicating if the operation is complex.
	* @return 0 on success, 1 if the job list is full.
	*/
	int job_insert(pid_t pid, char status, char* command, bool is_external, bool complex_op);

	/**
 	* @brief Removes a foreground job from the job array.
 	* @param pid Process ID of the job.
 	* @param status Status of the job.
 	*/
	void fg_job_remove(pid_t pid, char status);

	/**
	 * @brief Removes completed or stopped background jobs.
 	*/
    void job_remove();

	/**
 	* @brief Removes a complex job from the job array.
	*/
	void complexJob_remove();

	/**
 	* @brief Moves a job to the foreground.
 	* @param pid Process ID of the job to move to the foreground.
 	* @return 0 on success, 1 if the job was not found.
 	*/
	int job_2_front(int pid);
	// -------------------------------------------------
};
#endif