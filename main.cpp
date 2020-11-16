#include "minish.h"

const char* shell_name = "\nminish:";
//current working directory
char pwd[1000];
//current foreground process
pid_t cur_fg_pid = -1;
//current foreground command
char* cur_fg_cmd;
//current background command
char* cur_bg_cmd;
//Variable to check whether parent can wait or not
int can_wait = 1;
//previous exit status
int prev_exit_status = 0;
//job id
int job_id = 1;
//latest job id
bool isBg = false;
bool isRedirect = false;
bool isPipe = false;
int stdOutputFd = 0;
bool wildcard = false;

job_list *jl = (job_list*)malloc(sizeof(job_list));

//SIGINT signal handler
void sigint_handler(int sig_number){
	can_wait = 0;
	if(cur_fg_pid!=-1){
		kill(cur_fg_pid, SIGKILL);
		delete_process(jl, cur_fg_pid);
	}
	else{
		delete_process(jl, cur_fg_pid);
	}
	cur_fg_pid = -1;
	cur_fg_cmd = NULL;
	return;
}

//SIGCHLD handler for killing background process
void sigchld_handler(int sig_number){
	if(int proc_id = wait(&prev_exit_status)){
		if(proc_id==-1){
			return;
		}
		else if(search_process(jl, proc_id)){
			if(jl->head->pid == proc_id){
				jl->head->status = 0;
				printf("[%d] Done %20s\n", jl->head->jid, jl->head->cmd);
			}
			else{
				proc_info *tmp = jl->head;
				while(tmp->next_proc!= NULL && tmp->pid!=proc_id){
					tmp = tmp->next_proc;
				}
				if(tmp->pid==proc_id){
					tmp->status = 0;
				}
				printf("[%d] Done %20s\n", tmp->jid, tmp->cmd);
			}
			jl->size--;
		}
	}
}

//put background process in the job list
void insert(pid_t pid, int jid, int status, const char* line){
	proc_info *node = (proc_info*)malloc(sizeof(proc_info));
    node->pid = pid;
    node->jid = jid;
    node->status = status;
	node->cmd = (char*)malloc(sizeof(char));
	node->cmd = line;
    node->next_proc = NULL;
	if(jl->head==NULL){
		jl->head = (proc_info*)malloc(sizeof(proc_info));
		jl->tail = (proc_info*)malloc(sizeof(proc_info));
		jl->head = node;
		jl->tail = node;
	}
	else{
		proc_info *temp = jl->head;
		while(temp->next_proc!=NULL){
			temp = temp->next_proc;
		}
		if(temp->next_proc==NULL){
			temp->next_proc = (proc_info*)malloc(sizeof(proc_info));
			temp->next_proc = node;
			jl->tail = (proc_info*)malloc(sizeof(proc_info));
			jl->tail = node;
		}
	}
	++jl->size;
}

//Execute a command
void execute(char **args, bool bg_check){
	isBg = false;
	int child_pid = fork();
	if(child_pid<0){
		perror("ERROR: Failed to fork process\n");
		return;
	}
	if(child_pid==0){ //child process
		char arg1[100];
		strcpy(arg1, "/bin/");
		strcat(arg1, args[0]);
		int exec_res = execvp(arg1, args);
		if(exec_res==-1){
			perror("ERROR: Command failed\n");
		}
		if(!bg_check){
			exit(exec_res);
		}
	}
	else{ //parent process
		//If Background process
		if(bg_check){
			int bg_pid = child_pid;
			cur_bg_cmd = (char*)malloc(sizeof(char));
			int i = 1;
			strcpy(cur_bg_cmd,args[0]);
			while(!(args[i]==0)){
				strcat(cur_bg_cmd," ");
				strcat(cur_bg_cmd, args[i]);
				i++;
			}
			insert(bg_pid, job_id, 1, cur_bg_cmd);
			job_id++;
		}
		else{
			cur_fg_pid = child_pid+1;
			if(cur_fg_cmd == NULL){cur_fg_cmd = (char*)malloc(sizeof(char));}
			strcpy(cur_fg_cmd,args[0]);
			while(can_wait==1&&waitpid(child_pid, &prev_exit_status, WUNTRACED)>0){}
			cur_fg_pid = -1;
			cur_fg_cmd = NULL;
		}
	}
}

//Redirection
void redirect(const char *dest){
    int fd = open(dest,O_CREAT|O_TRUNC|O_WRONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
    // int bk = open("dummy",O_CREAT|O_TRUNC|O_WRONLY,S_IRUSR|S_IWUSR);
    if ((stdOutputFd = dup(1)) == -1) { 
        perror("Error");
        return;
    }
    if (dup2(fd, 1) == -1) { 
        perror("Error");
        // close(stdOutputFd);
        return;
    }
    close(fd);
}

//After Process of Redirection
void reverseredirect(){
	if (dup2(stdOutputFd, 1) == -1) {
    	printf("fd dup failed!!\n");
        exit(0);
    }
    close(stdOutputFd);
	isRedirect = false;
}

struct command{
	char **argv;
};

//pipe execution
void spawn_proc(int in, int out, struct command *cmd){
	int pid;
	if ((pid = fork ()) == 0){
    	if(in != 0){
    	    dup2 (in, 0);
        	close (in);
    	}
    	if(out != 1){
        	dup2 (out, 1);
        	close (out);
        }
    	int tmp = execvp(cmd->argv[0], cmd->argv);
    	exit(tmp);
    }
    else if(pid>0){
    	wait(&prev_exit_status);
    }
}

void fork_pipes(int n, struct command *cmd){
	int i;
	pid_t pid;
	pid_t child;
	int in, fd[2];
	in = 0;
	if((child = fork()==0)){
  		for (i = 0; i < n - 1; ++i){
    		if(pipe(fd)==-1){
    		perror("error pipe");
    	}
    	spawn_proc(in, fd[1], cmd + i);
    		close(fd[1]);
    		in = fd[0];
    	}
		if (in != 0){
			dup2 (in, 0);
		}
		execute(cmd[i].argv, false);
		// close(in);
		close(fd[0]);
		close(fd[1]);
		exit(0);
	}
	else if(child>0){
		int status;
		wait(&status);
		return;
	}
}


char **parsecmd(char *line){
	int buffer = MAX_BUFFER;
    char **args = (char **)malloc(buffer * sizeof(char*));
    char *arg;
    char *redirectiondest = (char *)malloc(buffer * sizeof(char));
    // char *pipedest = (char *)malloc(buffer * sizeof(char));
    int pos = 0;
    if (!args) {
        perror("Mem alloc error - args\n");
        exit(1);
    }
    arg = strtok(line, DELIMS);
    if(arg==NULL){return NULL;}
    while (arg != NULL) {
    	if(strcmp(arg, "&")==0){
    		isBg = true;
    		break;
    	}
    	else if(strcmp(arg,"|")==0){
    		isPipe = true;
    		arg = strtok(NULL, DELIMS);
    		continue;
    	}
    	else if(strcmp(arg, ">")==0){
    		isRedirect = true;
    		arg = strtok(NULL, DELIMS);
    		continue;
    	}
    	else{
    		if(isRedirect){
    			strcpy(redirectiondest, arg);
    		}
    		else if(isPipe){
				strcpy(redirectiondest, arg);
    		}
    		else{
    			for(int i=0;i<strlen(arg);i++){
    				if(arg[i]=='*'||arg[i]=='?'){
    					wildcard = true;
    				}
    			}
    			if(wildcard){
    				DIR *dr = opendir(".");
    				struct dirent *de;
    				int cnt = 0;
    				while((de = readdir(dr))!=NULL){
    					if(!fnmatch(arg, de->d_name,0)){
    						args[pos] = de->d_name;
    						pos++;
    						cnt++;
    						// printf("%s\n", de->d_name);
    					}
    				}
    				if(cnt==0){
    					perror("There are no such files.");
    					wildcard = false;
    					return NULL;
    				}
    				wildcard = false;
    			}
    			else{
        			args[pos] = arg;
        			pos++;
        		}
        	}	
    	}
        if (pos >= buffer) {
            buffer += MAX_BUFFER;
            args = (char **)realloc(args, buffer * sizeof(char *));
            if (!args) {
                perror("Mem realloc error - args");
                exit(2);
            }
        }
        // get next
        arg = strtok(NULL, DELIMS);
    }
    args[pos] = 0; 
    if(isRedirect){
    	// printf("%s", redirectiondest);
    	redirect(redirectiondest);
    	free(redirectiondest);
    }
    else if(isPipe){
    	char *second[] = {redirectiondest, 0};
    	struct command cmd [] = {args, second};
    	fork_pipes(2, cmd);
    	isPipe = false;
    	return NULL;
    }
    return args;
}

void Show_Jobs(){
	if(jl->head == NULL) return;
	else{
		proc_info *tmp = jl->head;
		switch(tmp->status){
			case 0:
				printf("[%d] Done %20s\n", tmp->jid, tmp->cmd);
				break;
			case 1:
				printf("[%d] Running %20s\n", tmp->jid, tmp->cmd);
				break;
			case 2:
				printf("[%d] Terminated %20s\n", tmp->jid, tmp->cmd);
				break;
			default:
				break;
		}
		while(tmp->next_proc!=NULL){
			tmp = tmp->next_proc;
			switch(tmp->status){
				case 0:
					printf("[%d] Done %20s\n", tmp->jid, tmp->cmd);
					break;
				case 1:
					printf("[%d] Running %20s\n", tmp->jid, tmp->cmd);
					break;
				case 2:
					printf("[%d] Terminated %20s\n", tmp->jid, tmp->cmd);
					break;
				default:
					break;
			}	
		}
	}
}

void cmd_executor(char *cmd){
	char **args;
	args = parsecmd(cmd);
	if(args==NULL){return;}
	else if(strcmp(args[0],"quit")==0){
		cur_fg_pid = clear_process(jl);
		printf("Exiting Shell\n");
		exit(0);
	}
	else if(strcmp(args[0],"cd")==0||strcmp(args[0],"path")==0||
			strcmp(args[0],"status")==0||strcmp(args[0],"kjob")==0){
		prev_exit_status = Built_In_Cmd(jl, pwd, args, prev_exit_status);
	}
	else if(strcmp(args[0],"jobs")==0){
		Show_Jobs();
	}
	else{
		if(args[0]!=NULL){
			execute(args,isBg);
		}
	}
	if(isRedirect){
		reverseredirect();
	}
	free(args);
}

void shell(){
	char *line = NULL;
	while(1){
		can_wait = 1;
		char fin_shell_name[strlen(shell_name)+strlen(pwd)+2];
		strcpy(fin_shell_name, shell_name);
		strcat(fin_shell_name, pwd);
		strcat(fin_shell_name, "$ ");
		printf("\n%s",fin_shell_name);
		size_t len = 0;
		if(getline(&line, &len, stdin)==-1){
			perror("ERROR: Reading cmd");
		}
		cmd_executor(line);
	}
}

int main(){
	init_job_list(jl);
	sigset_t newmask, oldmask;
    sigfillset(&newmask);
    sigdelset(&newmask, SIGINT);
    sigdelset(&newmask, SIGCHLD);
    sigprocmask(SIG_SETMASK, &newmask, &oldmask);
    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);
	strcpy(pwd, getcwd(pwd, 1000));
	shell();
	return 0;
}