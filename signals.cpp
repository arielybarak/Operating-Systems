// signals.c

#include <iostream>
#include "classes.h"
#include "signals.h"
#include <sys/wait.h>

using std::FILE;
using std::string;
using std::cout;
using std::cerr;
using std::prev;
extern job_arr job_list;

#define FG   '1'
#define BG 	 '2'
#define STOPPED '3'

// Reaps a terminated child process (without waiting)
// void handleFinishChld(int sig) {
//     sigset_t maskSet, oldSet;										 //TODO check that "sigemptyset" is not nececcery (and course's staff are assholes)
// 	sigfillset(&maskSet);
// 	sigdelset(&maskSet, SIGINT);		// Unmask Ctrl+C
//     sigdelset(&maskSet, SIGTSTP);		// Unmask Ctrl+Z //TODO think again if its not a problem (will the handler continue after SIGINT?)
// 	sigprocmask(SIG_SETMASK, &maskSet, &oldSet);
// 	int status;
//     pid_t pid = waitpid(-1, &status, WNOHANG);
// 	// cout << "handler is here: ";
// 	if(WIFEXITED(status)){ 							//WIFEXITED determines if a child exited with exit()
// 		cout << "child exited with exit() ";
// 		int exitStatus = WEXITSTATUS(status);
// 		if(exitStatus != 0)	//exit status != 0, handle error			//TODO continue errors
// 			cout << "but with error. CHECK IT NOW!\n";	
// 		else			//exit status == 0, handle success
// 			cout << "seccesfully\n";
// 	}
// 	job_list.job_remove(pid);
// 	sigprocmask(SIG_SETMASK, &oldSet, &maskSet);
// }

//handler for treating ctrl+c
void handle_ctrl_c(int sig) {
	sigset_t maskSet, oldSet;		
	sigfillset(&maskSet);
	sigprocmask(SIG_SETMASK, &maskSet, &oldSet);

	cout << "\nsmash: caught CTRL+C\n";
	if((job_list.jobs[0].is_external == 1) && (job_list.jobs[0].full == true))
	{
		if(!kill(job_list.jobs[0].pid, SIGKILL))
		{ 
    		cout << "process " << job_list.jobs[0].pid << " was killed\n";
			// job_list.fg_job_remove();
		} else {
   	 	perror("Failed to send SIGKILL\n\nsmash: ");
	}
	}
	sigprocmask(SIG_SETMASK, &oldSet, &maskSet);
}

//handler for treating ctrl+z
void handle_ctrl_z(int sig) {
	sigset_t maskSet, oldSet;		
	sigfillset(&maskSet);
	sigprocmask(SIG_SETMASK, &maskSet, &oldSet);

	cout << "\nsmash: caught CTRL+Z\n";
	if(job_list.jobs[0].full && job_list.jobs[0].is_external){

		if (!kill(job_list.jobs[0].pid, SIGSTOP)) {
			cout << "smash: process " << job_list.jobs[0].pid << " was stopped" << std::endl;
			job_list.fg_job_remove(job_list.jobs[0].pid, 1);
		}
		else {
			std::perror("smash error: kill failed");
			return;
		}
	}
	sigprocmask(SIG_SETMASK, &oldSet, &maskSet);
}

void MainHandleConfigPack(){
	// struct sigaction sa;
    // sa.sa_handler = &handleFinishChld;
    // sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;  							//NOCLDSTOP: Prevents SIGCHLD from being delivered when child processes stop 
    // sigaction(SIGCHLD, &sa, nullptr);

	struct sigaction sb;
    sb.sa_handler = &handle_ctrl_c;
	sb.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sb, nullptr);

	struct sigaction sc;
    sc.sa_handler = &handle_ctrl_z;
	sc.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &sc, nullptr);
}