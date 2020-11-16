#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <regex.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <fnmatch.h>

#define MAX_ARGS 16
#define MAX_BUFFER 25
#define DELIMS " \t\r\n\a"

//process structure
struct proc_info{
	pid_t pid; //process id
	int jid; //job id
	int status; //process status
	//0: Done, 1: Running, 2:Ter
	const char* cmd; //command
	struct proc_info *next_proc;
};

//process job list
struct job_list{
	proc_info *head;
	proc_info *tail;
	int size;
};

void cmd_executor(char *cmd);
void kill_bgprocess(job_list *jl, int jid, int prev_exit_status);
void init_job_list(job_list* jl);
int clear_process(job_list* jl);
int search_process(job_list* jl, pid_t pid);
void delete_process(job_list* jl, pid_t pid);
int Built_In_Cmd(job_list* jl, char *pwd, char **args, int prev_exit_status);
void Show_Jobs();