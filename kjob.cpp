#include "minish.h"

void kill_bgprocess(job_list *jl, int jid, int prev_exit_status){
	int tmp_pid;
	if(jl->head==NULL) return;
	else if(jid<1) return;
	else if(jl->head == jl->tail){ //if only one process
		if(jl->head->jid == jid){
			printf("[%d] Terminated %20s\n", jl->head->jid, jl->head->cmd);
			tmp_pid = jl->head->pid;
			jl->head=NULL;
			jl->tail=NULL;
			free(jl->head);
		}
		else return;
	}
	else if(jl->head->jid == jid){
		proc_info *tmp = jl->head;
		jl->head = jl->head->next_proc;
		tmp->next_proc=NULL;
		tmp_pid = tmp->pid;
		printf("[%d] Terminated %20s\n", tmp->jid, tmp->cmd);
		free(tmp);
	}
	else{
		proc_info *tmp = jl->head;
		proc_info *prev = jl->head;
		while(tmp->next_proc!= NULL && tmp->jid!=jid){
			prev = tmp;
			tmp = tmp->next_proc;
		}
		if(tmp->jid==jid){
			tmp_pid = tmp->pid;
			if(tmp == jl->tail){
				prev->next_proc = NULL;
				jl->tail = prev;
				printf("[%d] Terminated %20s\n", tmp->jid, tmp->cmd);
				free(tmp);
			}
			else{
				prev->next_proc = tmp->next_proc;
				tmp->next_proc=NULL;
				printf("[%d] Terminated %20s\n", tmp->jid, tmp->cmd);
				free(tmp);
			}
		}
		else return;
	}
	kill(tmp_pid, SIGKILL);
	prev_exit_status = 2;
	jl->size -= 1;
}