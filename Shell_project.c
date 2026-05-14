/**
UNIX Shell Project

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.

To compile and run the program:
   $ gcc Shell_project.c job_control.c -o Shell
   $ ./Shell          
	(then type ^D to exit program)

**/
#include <string.h>


#include "job_control.h"   // remember to compile with module job_control.c 

#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */


job *jobs; // Points to the memory where the list items are stored

// -----------------------------------------------------------------------
//                            MAIN          
// -----------------------------------------------------------------------

void handle_sigchld(int sig) {
	printf("num de jobs: %d", list_size(jobs));

	//print_job_list(jobs);
	for(int jobi = list_size(jobs); jobi >= 0; jobi--) {
		job* thisJob = get_item_bypos(jobs, jobi);
		if(thisJob == NULL) continue;

		int status;

		int pid_wait = waitpid(thisJob->pgid, &status, WNOHANG | WUNTRACED);
		if(pid_wait == 0) continue; // No state change for this job

		if(WIFEXITED(status) || WIFSIGNALED(status)) {
			delete_job(jobs, thisJob);
		} else if(WIFSTOPPED(status)) {
			thisJob->state = STOPPED;
		}
	}
}

int main(void)
{
	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	int background;             /* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE/2];     /* command line (of 256) has max of 128 arguments */
	// probably useful variables:
	int pid_fork, pid_wait; 	/* pid for created and waited process */
	int status;             	/* status returned by wait */
	char *file_in, *file_out; 	/* file names for redirection */

	jobs = new_list("hola");

	const char *homeDir = getenv("HOME");

	ignore_terminal_signals();
	

	signal(SIGCHLD, handle_sigchld);

	while (1)   /* Program terminates normally inside get_command() after ^D is typed*/
	{   		
		

		printf("COMMAND->");
		fflush(stdout);
		get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */
		
		if(args[0]==NULL) continue;   // if empty command

		// internal commands
		if(!strcmp(args[0], "cd")){
			if(args[1] == NULL || !strcmp(args[1], "~")){
				chdir(homeDir);
				continue;
			}

			chdir(args[1]);
			continue; //replace by return when refactoring to a function
		}

		if(!strcmp(args[0], "jobs")){
			print_job_list(jobs);
			continue; //replace by return when refactoring to a function
		}
		
		//	 (1) fork a child process using fork()
		pid_fork = fork();

		if(pid_fork < 0){
			printf("woops, the shell couldn't create the process (fork failed)\n");
			continue;
		}

		
		//	 (2) the child process will invoke execvp()
		if (pid_fork == 0) { // child process
			setpgid(0,getpid());

			if(!background){
				tcsetpgrp(STDIN_FILENO, getpgrp());
			}

			restore_terminal_signals();

			execvp(args[0],args);
			printf("woops, the shell couldn't create the process (exec failed)\n");
			exit(1);
		}


		//	 (3) if background == 0, the parent will wait, otherwise continue 
		if(!background){
			waitpid(pid_fork,&status, WUNTRACED);
			tcsetpgrp(STDIN_FILENO, getpgrp());

			if(WIFSTOPPED(status)){
				add_job(jobs, new_job(pid_fork, args[0], STOPPED));
			}
		} else {
			add_job(jobs, new_job(pid_fork, args[0], BACKGROUND));
		}
		
		//	 (4) Shell shows a status message for processed command 
		//	 (5) loop returns to get_commnad() function
		

	} // end while
}
