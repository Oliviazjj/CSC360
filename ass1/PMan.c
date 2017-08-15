#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_PROC 4096
#define MAX_CMD 4096

int proc_count=0;
/*Object: struct proc*/
typedef struct {
    pid_t pid;		/*process id*/
    char *cmd;		/*char *cmd - command and arguments*/
    bool isStop;	/*bool isStop - whether process is stopped.*/
} proc;

proc *proc_list[MAX_PROC];
void update_bg_process() {

	int i;
	for(i=0; i<proc_count; i++){
		int status;
		int id=waitpid(proc_list[i]->pid, &status, WNOHANG); 
		if(id> 0){
          		//alert the user to the end of this process
          		printf("Process with PID %ld terminated\n", (long)proc_list[i]->pid);
         		proc_count--;
			proc_list[i]=proc_list[proc_count];
			
    			 		
        	}
		
		
    	}  
	
}

main(){


	char *input = NULL ;
    	char *prompt = "PMan: >";
	int   pid_cmd;/*get pid from cmd*/
	char* command[MAX_CMD];
	int cmd_count;
	char *s;
   	 while(input = readline(prompt)){
		cmd_count=0;
		char first_com[1024];
		s=strtok(input," ");
		if(s!=NULL){
			strcpy(first_com,s);
		}
		while(s!=NULL){
			command[cmd_count]=s;
			s=strtok(NULL," ");
			cmd_count++;
		}  
		//printf("first: %s, second: %s: ",command[0],command[1]);   	
		if(strcmp(command[0], "bgkill") == 0){
			if(cmd_count==2){
				pid_cmd=atoi(command[1]);
    				kill(strtol(command[1],NULL,0),SIGTERM);
    				printf("kill the process %s\n",command[1]);
			}else{
				printf("not follow the format: bgkill pid\n");
				continue;
			}
			update_bg_process();
		}else if(strcmp(command[0], "bgstart") == 0){
			if(cmd_count==2){
				pid_cmd=atoi(command[1]);
    				kill(strtol(command[1],NULL,0),SIGCONT);
    				printf("restart the process %s\n",command[1]);
			}else{
				printf("not follow the format: bgstart pid\n");
				continue;
			}
			update_bg_process();
    		}else if(strcmp(command[0], "bgstop") == 0){
			if(cmd_count==2){
				pid_cmd=atoi(command[1]);
    				kill(strtol(command[1],NULL,0),SIGSTOP);
    				printf("stop the process %s\n",command[1]);
			}else{
				printf("not follow the format: bgstop pid\n");
				continue;
			}
			update_bg_process();
        	}else if (strcmp(command[0],"bg")==0) {
			if(cmd_count <2){
				printf("Not follow the format: bg com\n");
				continue;
			}			
			int pid = fork();
			if(pid==0){
				if(execvp(command[1],command)== -1){
					printf("Invaild command input\n");
					continue;
				}
			}else if( pid<0){
				printf("Failed fork\n");
				continue;
			}else{
				
				proc_list[proc_count]= malloc(sizeof(proc));
				proc_list[proc_count]->cmd=command[1];
    				proc_list[proc_count]->pid = pid;
    				proc_list[proc_count]->isStop = 0;
    				proc_count++;
				//wait(NULL);
				//int status;
				//while(wait(&status)!= pid);
				update_bg_process();
			}
			
        	} else if (strcmp(command[0], "pstat") == 0){
			if(cmd_count != 2){
				printf("not follow format: pstat pid\n");
				continue;
			}
			FILE *fp_stat;
			FILE *fp_status;
			char path_stat[1024];
			char filename[1000];
			sprintf(filename, "/proc/%s", command[1]);
    			if(fopen(filename, "r")==NULL){
				printf("Error: Process %s does not exist\n",command[1]);
				continue;
			}
    			sprintf(filename, "/proc/%s/stat", command[1]);
    			fp_stat = fopen(filename, "r");
			int stat_count=0;
    			if(fp_stat) {
        		     while(fgets(path_stat, sizeof(path_stat)-1, fp_stat) != NULL){
            			char *str;
            			str = strtok(path_stat," ");
            			while (str != NULL){	
                            		stat_count++;
					if(stat_count==1){
						printf("Display the information of process %s:\n",str);
					}
                           		else if(stat_count==2){
                                		printf("comm: %s\n",str);
                            		}else if(stat_count==3){
                                	printf("state: %s\n",str);
                            		}else if(stat_count==14){
                                		printf("utime: %lf\n",(atof(str))/sysconf(_SC_CLK_TCK));
                            		}else if(stat_count==15){
                               			printf("stime: %lf\n",(atof(str))/sysconf(_SC_CLK_TCK));
                            		}else if(stat_count==24){
                                		printf("rss: %s\n",str);
            				}
					str = strtok(NULL, " ");
        			}
			     }
    			} else {
        			perror("proc stat file cannot be opened\n");
        			exit(1);
    			}
   			if(pclose(fp_stat)==-1){
				printf("failed in closing the stream\n");
				continue;
			}
   			sprintf(filename, "/proc/%s/status", command[1]);
    			fp_status = fopen(filename, "r");
    			if(fp_status) {
                    	    	
				char* path_status;
				size_t length=0;
				char *token;
				
				while(getline(&path_status, &length, fp_status) != -1){

					token=strtok(path_status," :");
					//bool print_flag=0;
            				if(token!= NULL){
						char* string1 = "voluntary_ctxt_switches";
          					char* string2 = "nonvoluntary_ctxt_switches";
          					if(strcmp(token, string1) == 0)
            					{
             						token = strtok(NULL, " :\n");
              						printf("%s: %s\n", string1, token);
            					}
          					if(strcmp(token, string2) == 0)
            					{
              						token = strtok(NULL, " :\n");
              						printf("%s: %s\n", string2, token);
            					}
						//token=strtok(NULL,"\n");
						//printf("last one: %s",token);
						
					}
        				
    				} 
    			}
   			if(pclose(fp_status)==-1){
				printf("failed in closing the stream\n");
				continue;
			}
        	}	
		else if(strcmp(command[0], "bglist") == 0){
				update_bg_process();
				if(cmd_count != 1){
					printf("not follow the format: bglist\n");
					continue;
				};
				int i;
				printf("total background jobs: %d\n",proc_count);
				
				for(i=0; i<proc_count; i++){
					printf("%d: %s\n",proc_list[i]->pid,proc_list[i]->cmd);
				}

        	} else {
           		printf("command not found\n");
			continue;
			
        	}
		update_bg_process();
		
		
    	}
	return;

}
