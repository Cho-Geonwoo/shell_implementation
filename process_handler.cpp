#include "minish.h"

//initiate process job list
void init_job_list(job_list* jl){
	jl -> head = NULL;
	jl -> tail = NULL;
	jl -> size = 0;
}

//Remove all processes
int clear_process(job_list* jl){
	if(jl->size==0){
		return -1;
	}
	proc_info *tmp = jl->head;
	while(tmp!=jl->tail){
		proc_info *del_tmp = tmp;
		tmp = tmp->next_proc;
		free(del_tmp);
	}
	free(tmp);
	return -1;
}

//find a specified process in job list
int search_process(job_list* jl, pid_t pid){
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
void delete_process(job_list* jl, pid_t pid){
	if(jl->head==NULL) return;
	else if(pid==-1) return;
	else if(search_process(jl, pid)==0) return; //process not found
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