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

#define MAX_ARGS 16
#define MAX_BUFFER 25
#define DELIMS " \t\r\n\a"

const char* shell_name = "\nminish:";
//current working directory
char pwd[1000];
//current foreground process
pid_t cur_fg_pid = -1;
//current foreground command
char* cur_fg_cmd;
//current background process
int cur_bg_pid = -1;
//current background command
char* cur_bg_cmd;
//Variable to check whether parent can wait or not
int can_wait = 1;
//previous exit status
int prev_exit_status = 0;
//job id
int job_id = 0;
//latest job id
int latest_jobid = -1;
bool isBg = false;
bool isRedirect = false;
bool isPipe = false;
int bk;

void cmd_executor(char *cmd);

//process structure
struct proc_info{
   pid_t pid; //process id
   int jid; //job id
   int status; //process status
   //0: Done, 1: Running, 2:Ter
   char *cmd; //command
   struct proc_info *next_proc;
};

//process job list
struct job_list{
   proc_info *head;
   proc_info *tail;
   int size;
};

job_list *jl = (job_list*)malloc(sizeof(job_list));

//initiate process job list
void init_job_list(){
   jl -> head = NULL;
   jl -> tail = NULL;
   jl -> size = 0;
}

//find a specified process in job list
int search_process(pid_t pid){
   if(jl->head == NULL) return 0;
   else{
      proc_info *tmp = jl->head;
      while(tmp->next_proc!=NULL){
         if(tmp->pid==pid){
            return 1;
         }
         tmp = tmp->next_proc;
      }
      if(tmp->pid==pid) return 1;
      return 0;
   }
}

//delete process
void delete_process(pid_t pid){
   if(jl->head==NULL) return;
   else if(pid==-1) return;
   else if(search_process(pid)==0) return; //process not found
   else if(jl->head == jl->tail){ //if only one process
      if(jl->head->pid == pid){
         jl->head=NULL;
         jl->tail=NULL;
         free(jl->head);
      }
      else return;
   }
   else if(jl->head->pid == pid){
      proc_info *tmp = jl->head;
      jl->head = jl->head->next_proc;
      tmp->next_proc=NULL;
      free(tmp);
   }
   else{
      proc_info *tmp = jl->head;
      proc_info *prev = jl->head;
      while(tmp->next_proc!= NULL && tmp->pid!=pid){
         tmp = tmp->next_proc;
      }
      if(tmp->pid==pid){
         if(tmp == jl->tail){
            prev->next_proc = NULL;
            jl->tail = prev;
            free(tmp);
         }
         else{
            prev->next_proc = tmp->next_proc;
            tmp->next_proc=NULL;
            free(tmp);
         }
      }
      else return;
   }
   jl->size -= 1;
}

//SIGINT signal handler
void sigint_handler(int sig_number){
   can_wait = 0;
   if(cur_fg_pid!=-1){
      kill(cur_fg_pid, SIGKILL);
      delete_process(cur_fg_pid);
   }
   else{
      delete_process(cur_fg_pid);
   }
   cur_fg_pid = -1;
   cur_fg_cmd = NULL;
   // exit(0);
   return;
}

//Remove all processes
void clear_process(){
   if(jl->size==0){
      return;
   }
   proc_info *tmp = jl->head;
   while(tmp!=jl->tail){
      proc_info *del_tmp = tmp;
      tmp = tmp->next_proc;
      free(del_tmp);
   }
   free(tmp);
   cur_fg_pid = -1;
}

void insert(pid_t pid, int jid, int status, char *line){

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
      if(!bg_check) exit(exec_res);
   }
   else{ //parent process
      //If Background process
      if(bg_check){
         int status;
         int bg_pid = child_pid+1;
         insert(bg_pid, job_id, 2, args[0]);
         latest_jobid = job_id;
         cur_bg_pid = bg_pid;
         if(cur_bg_cmd == NULL){cur_bg_cmd = (char*)malloc(sizeof(char));}
         cur_bg_cmd =args[0];
         job_id++;
         // if(waitpid(child_pid, &prev_exit_status, WNOHANG)){
         //    printf("[%d]+ Done %-7s", job_id, cur_bg_cmd);
         //    job_id--;
         // }
      }
      else{
         int status;
         cur_fg_pid = child_pid+1;
         if(cur_fg_cmd == NULL){cur_fg_cmd = (char*)malloc(sizeof(char));}
         strcpy(cur_fg_cmd,args[0]);
         while(can_wait==1&&waitpid(child_pid, &status, WUNTRACED)>0){}
         if(WIFEXITED(status)){
            prev_exit_status = status;
         }
         cur_fg_pid = -1;
         cur_fg_cmd = NULL;
         latest_jobid = -1;
      }
   }
}

//Built in command
int Built_In_Cmd(char **args){
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
    else{
       printf("Previous exit status was %d\n", prev_exit_status);
    }
    return 0;
}

// bool isBg(char **args){
//    int i = 0;
//     while(args[i] != NULL) {
//         if (strcmp(args[i], "&") == 0) {
//            return 1;
//         }
//         i++;
//     }
//     return 0;
// }

void redirect(char *dest){
    int stdOutputFd;
    int fd = open(dest,O_CREAT|O_TRUNC|O_WRONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
    int bk = open("dummy",O_CREAT|O_TRUNC|O_WRONLY,S_IRUSR|S_IWUSR);
    if ((stdOutputFd = dup2(1, bk)) == -1) { 
        perror("Error");
        return;
    }
    if (dup2(fd, 1) == -1) { 
        perror("Error");
        close(stdOutputFd);
        return;
    }
    close(fd);
}

void reverseredirect(){
   if (dup2(bk, 1) == -1) {
       printf("fd dup failed!!\n");
        exit(0);
    }
   isRedirect = false;
   remove("dummy");
}

char **parsecmd(char *line){
   int buffer = MAX_BUFFER;
    char **args = (char **)malloc(buffer * sizeof(char*));
    char *arg;
    char *redirectiondest = (char *)malloc(buffer * sizeof(char));
    char *pipedest = (char *)malloc(buffer * sizeof(char));
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
       if(strcmp(arg,"|")==0){
          isPipe = true;
          arg = strtok(NULL, DELIMS);
          continue;
       }
       if(strcmp(arg, ">")==0){
          isRedirect = true;
          arg = strtok(NULL, DELIMS);
          continue;
       }
       if(isRedirect){
          strcat(redirectiondest, arg);
       }
       else if(isPipe){
         strcat(pipedest, arg);
       }
       else{
           args[pos] = arg;
           pos++;
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
    args[pos] = NULL; 
    if(isRedirect){
       redirect(redirectiondest);
    }
    else if(isPipe){
       char *tmp_file = "tmp";
       strcat(pipedest, tmp_file);
       redirect(tmp_file);
       int childpid;
       if((childpid = fork()) == -1)
        {
                perror("fork");
                exit(1);
        }
        if(childpid == 0)
        {
            char arg1[100];
         strcpy(arg1, "/bin/");
         strcat(arg1, args[0]);
         execvp(arg1, args);
          reverseredirect();
        }
        else
        {
           cmd_executor(pipedest);
           remove("tmp");
           isPipe = false;
          return NULL;
        }
    }
    free(redirectiondest);
    free(pipedest);
    return args;
}

void Show_Jobs(){
   if(cur_bg_pid==-1){
      return;
   }

}

void cmd_executor(char *cmd){
   char **args;
   args = parsecmd(cmd);
   if(args==NULL){return;}
   else if(strcmp(args[0],"quit")==0){
      clear_process();
      printf("Exiting Shell\n");
      exit(0);
   }
   else if(strcmp(args[0],"cd")==0||strcmp(args[0],"path")==0||strcmp(args[0],"status")==0){
      Built_In_Cmd(args);
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
      // free(line);
   }
}

int main(){
   init_job_list();
   // cmd_executor("clear");
   sigset_t newmask, oldmask;
    sigfillset(&newmask);
    sigdelset(&newmask, SIGINT);
    sigprocmask(SIG_SETMASK, &newmask, &oldmask);
    signal(SIGINT, sigint_handler);
   strcpy(pwd, getcwd(pwd, 1000));
   shell();
   return 0;
}