#include "minish.h"

//Built in command
int Built_In_Cmd(job_list* jl, char *pwd, char **args, int prev_exit_status){
	if (strcmp(args[0], "cd") == 0) {
		int tmp;
		if(args[1]==NULL){
			tmp = chdir(getenv("HOME"));
			strcpy(pwd, getcwd(pwd, 1000));
		}
        else{
        	tmp = chdir(args[1]);
        	strcpy(pwd, getcwd(pwd, 1000));
        }	
        if(tmp!=0){
        	perror("Error: Failed to change directory");
        }
    }
    else if(strcmp(args[0], "path")==0){
    	if(args[1]==NULL){
			char* path_var;
			path_var = getenv("PATH");
			printf("%s", path_var);
		}
		else{
			setenv("PATH",args[1],1);
		}
    }
    else if(strcmp(args[0],"kjob")==0){
    	kill_bgprocess(jl, atoi(args[1]), prev_exit_status);
    }
    else{
    	printf("Previous exit status was %d\n", prev_exit_status);
    }
    return prev_exit_status;
}