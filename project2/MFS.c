/*

time calculation may be a nightware! 
Beware of float, int, unsigned int conversion.
you could use gettimeofday(...) to get down to microseconds!

*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#define MAXFLOW 100



typedef struct 
{
    float arrivalTime ;
    float transTime ;
    int priority ;
    int id ;
} flow;

flow flowList[MAXFLOW];   // parse input in an array of flow
flow *queueList[MAXFLOW];  // store waiting flows while transmission pipe is occupied.
pthread_t thrList[MAXFLOW]; // each thread executes one flow
pthread_mutex_t trans_mtx = PTHREAD_MUTEX_INITIALIZER ; 
pthread_cond_t trans_cvar = PTHREAD_COND_INITIALIZER ; 
pthread_mutex_t queue_mtx = PTHREAD_MUTEX_INITIALIZER ; 
int num_flow=0;
int count_queue=0;
int trans_id; 
struct timeval start_time;


void requestPipe(flow *item) {
	pthread_mutex_lock(&trans_mtx);
    if(!trans_id){
		trans_id=item->id;
		pthread_mutex_unlock(&trans_mtx);
		return;
	}else if(trans_id==0 && count_queue==0){
		trans_id=item->id;
		pthread_mutex_unlock(&trans_mtx);
		return;
	}
	pthread_mutex_lock(&queue_mtx);
	//ADD THE ITEM INTO QUEUELIST AND SORT IT
	if(count_queue==0){
		queueList[0]=item;
	}else{
		int i;
		for(i=count_queue-1; i>=0; i--){
			if(item->priority==queueList[i]->priority){
				if(item->arrivalTime==queueList[i]->arrivalTime){
					if(item->transTime==queueList[i]->transTime){
						if(item->id<queueList[i]->id){
							int j;
							for(j=count_queue-1;j>i; j--){
								queueList[j+1]=queueList[j];
							}
							queueList[i+1]=item;
							break;
						}else{
							if(count_queue==1){
								queueList[1]=queueList[0];
								queueList[0]=item;
								break;
							}
							if(i==0){
								int j;
								for(j=count_queue; j>=1; j++){
									queueList[j]=queueList[j-1];
								}
								queueList[0]=item;
								break;
							}
						}
			
					}else if(item->transTime < queueList[i]->transTime){
						int j;
						for(j=count_queue-1;j>i; j--){
							queueList[j+1]=queueList[j];
						}
						queueList[i+1]=item;
						break;
					}else{
						if(count_queue==1){
							queueList[1]=queueList[0];
							queueList[0]=item;
							break;
						}
						if(i==0){
							int j;
							for(j=count_queue; j>=1; j++){
								queueList[j]=queueList[j-1];
							}
							queueList[0]=item;
							break;
						}
					}
				}else if(item->arrivalTime<queueList[i]->arrivalTime){
					int j;
					for(j=count_queue-1;j>i; j--){
						queueList[j+1]=queueList[j];
					}
					queueList[i+1]=item;
					break;
				}else{
					if(count_queue==1){
						queueList[1]=queueList[0];
						queueList[0]=item;
						break;
					}
					if(i==0){
						int j;
						for(j=count_queue; j>=1; j++){
							queueList[j]=queueList[j-1];
						}
						queueList[0]=item;
						break;
					}
				}
			}else if(item->priority<queueList[i]->priority){
				int j;
				for(j=count_queue-1;j>i; j--){
					queueList[j+1]=queueList[j];
				}
				queueList[i+1]=item;
				break;
			}else{
				if(count_queue==1){
					queueList[1]=queueList[0];
					queueList[0]=item;
					break;
				}
				if(i==0){
					int j;
					for(j=count_queue; j>=1; j++){
						queueList[j]=queueList[j-1];
					}
					queueList[0]=item;
					break;
				}
			}
		}
		
	}
	int j;
	for(j=0; j<=count_queue; j++){
		//printf("here is the list: %d %d\n",j,queueList[j]->id);
	}
	count_queue++;
	printf("Flow %2d waits for the finish of flow %2d. \n",item->id,trans_id);
	pthread_mutex_unlock(&queue_mtx);   
	
    while(trans_id!=item->id){
		pthread_cond_wait(&trans_cvar,&trans_mtx);
		pthread_mutex_lock(&queue_mtx);
		trans_id=queueList[count_queue-1]->id;
		pthread_mutex_unlock(&queue_mtx);	
	}
	pthread_mutex_lock(&queue_mtx); 	
    count_queue--;
	if(count_queue==0){
		trans_id=0;
	}
	queueList[count_queue]=NULL;
	pthread_mutex_unlock(&queue_mtx);
}
//get timestamp in second

float get_time(){
	struct timeval cur;
	gettimeofday(&cur,NULL);
	float time = (cur.tv_sec - start_time.tv_sec)+((cur.tv_usec - start_time.tv_usec)/1000000.0);
	return time;
}
void releasePipe() {
	pthread_mutex_unlock(&trans_mtx);	
	pthread_cond_broadcast(&trans_cvar);	
}

// entry point for each thread created
void *thrFunction(void *flowItem) {

    flow *item = (flow *)flowItem ;

    // wait for arrival
    usleep((item->arrivalTime)*1000000);
	printf("Flow %2d arrives: arrival time (%.2f), transmission time (%.1f), priority (%2d). \n",item->id,get_time(),item->transTime, item->priority);
    requestPipe(item) ;
    printf("Flow %2d starts its transmission at time %.2f. \n",item->id,get_time());

    // sleep for transmission time
    usleep((item->transTime) *1000000);
    releasePipe(item) ;
    printf("Flow %2d finishes its transmission at time %0.1f. \n",item->id, get_time());
}
int main(int argc, char *argv[]) {

	if(argc!=2){
		printf("Failed in argument\n");
		return -1;
	}
    //file handling
	
    FILE *fp = fopen(argv[1],"r");// read fild
	if(fp==NULL){
		printf("failed in reading file\n");
		return -1;
	}
    // read number of flows
	if(!(fscanf(fp,"%d\n",&num_flow))){
		printf("check your input file, the first line is the number of flows");
		return -1;
	}
	int i;
	float arrival,transmission;
    for(i=0; i<num_flow; i++) {
        if(!(fscanf(fp,"%d:%f,%f,%d\n",&flowList[i].id,&arrival,&transmission,&flowList[i].priority))){
			printf("failed in input format. For each row, follow: id:arrival time,transmission time, priority");
		}
		flowList[i].arrivalTime=arrival/10;
		flowList[i].transTime=transmission/10;
    }

    fclose(fp); // release file descriptor
	gettimeofday(&start_time, NULL);
    for(i=0; i<num_flow; i++){
        pthread_create(&thrList[i], NULL, thrFunction, (void *)&flowList[i]) ;
	}
    // wait for all threads to terminate
	for(i=0; i<num_flow; i++){
    	pthread_join(thrList[i],NULL);
	}
    // destroy mutex & condition variable
	pthread_mutex_destroy(&trans_mtx);
	pthread_mutex_destroy(&queue_mtx);
	pthread_cond_destroy(&trans_cvar);

    return 0;
}
