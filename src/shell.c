/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: t; -*- */
/*
 * Minish shell.
 *
 * Add The name of the first team member here in the form Surname Lastname, iis...
 * Add The name of the first team member here in the form Surname Lastname, iis...
*/

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parser.h"

#define INPUT_SIZE 4096
#define MAX_JOBS 128

struct job {
	pid_t pid;
	int active;
};

// This array can hold information about spawned processes.
// Each array element describes a new process and holds its
// PID and its state in the active variable. This variable
// may take the value 1 if the job is active, or the value
// 0 if the job is not active. The array can be used to
// manage background jobs.
//
// You may use this array if you find it helpful. Otherwise,
// fill free to delete it.
static struct job jobs[MAX_JOBS];

static void print_prompt(void);
static int read_line(char *buffer, size_t size);
static int execute_pipeline(const struct pipeline *pipeline);
static void init_bg_jobs(struct job *jobs);
static void add_bg_job(const pid_t pid);
static void remove_bg_job(const pid_t pid);
static void reap_bg_jobs(void);
static void handle_sigchld(int sig);

// Add the prototypes of helper funcitons here...

/**
 * @brief Run the minish command interpreter.
 *
 * @return Process exit status.
 */
int main(void)
{	
	char line[INPUT_SIZE];
	struct pipeline pipeline;
	int error;
	int result;

	init_bg_jobs(jobs);

	// Install the required signal handlers here.
	struct sigaction sa={0};

	sa.sa_handler=handle_sigchld;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags=SA_RESTART | SA_NOCLDSTOP;

	if(sigaction(SIGCHLD,&sa,NULL)==-1){
		perror("sigaction");
		return EXIT_FAILURE;
	}
	struct sigaction int_action={0};
	int_action.sa_handler=SIG_IGN;
	sigemptyset(&int_action.sa_mask);

	if(sigaction(SIGINT,&int_action,NULL)==-1){
		perror("sigaction");
		return EXIT_FAILURE;
	}
	 
	// The main shell loop.
	for (;;) {
		// Print the prompt.
		print_prompt();

		// Read a command line from the user.
		result = read_line(line, sizeof(line));

		// Handle read errors.
		if (result == 0)
			break;

		if (result < 0) {
			if (errno == EINTR)
				continue;

			perror("minish: Cannot read the command line.");
			return EXIT_FAILURE;
		}

		// Parse the line.
		error = parse_line(line, &pipeline);

		// An error occurred during parsing.
		if (error != PARSE_SUCCESS) {
			fprintf(stderr, "minish: %s\n", parse_error_string(error));
			continue;
		}

		// The user just pressed enter. We have nothing to execute at this time.
		if (pipeline.command_count == 0) {
			pipeline_free(&pipeline);
			continue;
		}

		// Execute the command pipeline.
		result=execute_pipeline(&pipeline);
		if(result== -2){
			pipeline_free(&pipeline);
			exit(0);
		}
		else if(result == -1){
			perror("minish");
		}

		// Clear the pipeline.
		pipeline_free(&pipeline);
	}
	return EXIT_SUCCESS;
}

/**
 * @brief Print the interactive shell prompt.
 */
static void print_prompt(void)
{
	if (isatty(STDIN_FILENO) != 0) {
		(void)printf("minish$ ");
		(void)fflush(stdout);
	}
}

/**
 * @brief Read one command line from standard input.
 *
 * @param[out] buffer Destination buffer.
 * @param[in] size Size of the destination buffer.
 *
 * @return 1 when a line was read, 0 on EOF, or -1 on error.
 */
static int read_line(char *buffer, size_t size)
{
	if (fgets(buffer, size, stdin) == NULL) {
		if (feof(stdin) != 0)
			return 0;

		return -1;
	}

	buffer[strcspn(buffer, "\n")] = '\0';

	return 1;
}

void init_bg_jobs(struct job *jobs)
{
	int i;
  
	if (!jobs)
		return;
		
	for (i = 0; i < MAX_JOBS; i++) {		
		jobs[i].pid =-1;
		jobs[i].active = 0;
	}
}

void add_bg_job(const pid_t pid)
{
	int i;

	for (i = 0; i < MAX_JOBS; i++) {
		// Find the first position in the array with active == 0 and add the
		// new job there. Then call break to exit the loop.
        if(jobs[i].active==0){
			jobs[i].pid=pid;
			jobs[i].active=1;
			break;
		}
	}
}

void remove_bg_job(const pid_t pid)
{
	int i;

	for (i = 0; i < MAX_JOBS; i++) {
		// Find the first position in the array where its pid matches with the
		// pid parameter of the function and then clean it by setting pid to
		// -1 and active to 0.
        if(jobs[i].active && jobs[i].pid==pid){
			jobs[i].pid=-1;
			jobs[i].active=0;
			break;
		}
	}
}

// The function below should be called from a signal handler.
void reap_bg_jobs(void) {
	pid_t pid;
	int status;	
	int i;

	for (i = 0; i < MAX_JOBS; i++) {
		// Call pid = waitpid() system call properly for each array slot.
		// If this call returns a positive pid, then this means that we just
		// found a terminated background process. Call remove_bg_job()
		// properly to remove this process from the jobs array.
          		
        if(jobs[i].active){
			pid=waitpid(jobs[i].pid,&status,WNOHANG);

			if(pid>0||(pid==-1 &&errno==ECHILD)){
				remove_bg_job(jobs[i].pid);
			}
		}
	}	
}
static void handle_sigchld(int sig){
	(void)sig;
	int saved_errno=errno;
	reap_bg_jobs();
	errno=saved_errno;
 }


/**
 * @brief Execute all commands in a parsed pipeline.
 *
 * @param[in] pipeline Parsed pipeline.
 *
 * @return 0 on success, -1 on error, -2 if exit was requested.
 */


 


static int execute_pipeline(const struct pipeline *pipeline)
{
	pid_t *pids;	
	int result = 0;
	
	// First, we have to handle the internal commands. To do this, we need to
	// check if our command count is one, and if the name of the command matches
	// an internal command. Remember that the command name is always stored in
	// argv[0].

	if(pipeline->command_count==1){
		const struct command *cmd= &pipeline->commands[0];
		if(cmd->argc>0 && strcmp(cmd->argv[0],"exit")==0){
			return -2;
		}
		if(cmd->argc>0 && strcmp(cmd->argv[0],"cd")==0){
			const char *path;
			if(cmd->argc>1){
				path=cmd->argv[1];
			}
			else{
				path=getenv("HOME");
			}
			if(path==NULL || chdir(path)== -1){
				perror("cd");
				return -1;
			}
			return 0;
		}
	}

	// No internal command found. We have to execute a new program for each
	// command in the pipeline.

	// Allocate an array to store the PID of each child process.
    // You may use the array below, if you find it helpful.    
	pids = calloc(pipeline->command_count, sizeof(*pids));
	if (pids == NULL)
		return -1;

	// Process each command in the pipeline. Remember that the command_count
	// tells us the number of commands in the pipeline.
	// E.g., cmd1 | cmd2 : 2 commands.
	// E.g., cmd1 : 1 command.
	sigset_t block_set,old_set;

	sigemptyset(&block_set);
	sigaddset(&block_set,SIGCHLD);

	if(sigprocmask(SIG_BLOCK,&block_set,&old_set)==-1){
		perror("sigprockmask");
		free(pids);
		return -1;
	}

	for(size_t i=0; i<pipeline->command_count;i++){
		pid_t pid=fork();
		if(pid<0){
			perror("fork");
			sigprocmask(SIG_SETMASK,&old_set,NULL);
			free(pids);
			return -1;
		}
		if (pid==0){
			if(sigprocmask(SIG_SETMASK,&old_set,NULL)==-1){
				perror("sigprocmask");
				_exit(127);
			}

			if(!pipeline->background){
				struct sigaction dfl={0};

				dfl.sa_handler=SIG_DFL;
				sigemptyset(&dfl.sa_mask);

				if(sigaction(SIGINT,&dfl,NULL)==-1){
					perror("sigaction");
					_exit(127);
				}
			}

			struct command *cmd= &pipeline->commands[i];
			execvp(cmd->argv[0],cmd->argv);
			perror("execvp");
			_exit(127);
		}
		pids[i]=pid;

		if(pipeline->background){
			add_bg_job(pid);
		}
	}
	if(!pipeline->background){
		for(size_t i=0;i<pipeline->command_count;i++){
			waitpid(pids[i],NULL,0);
		}
	}
    
	sigprocmask(SIG_SETMASK,&old_set,NULL);
	
	free(pids);
	return result;
}
