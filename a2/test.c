#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>

#include "llist.h"
#include "queue.h"

#define CLERK_AMNT 4

void add_customer_details(char* str);
void * customer_entry(void * cus_info);
void *clerk_entry(void * clerkNum);
/**
 Skeleton code of assignment 2 (For reference only)
	   
 The time calculation could be confused, check the exmaple of gettimeofday on connex->resource->tutorial for more detail.
 */

/* global variables */
 
static struct timeval init_time; // use this variable to record the simulation start time; No need to use mutex_lock when reading this variable since the value would not be changed by thread once the initial time was set.
double overall_waiting_time; //A global variable to add up the overall waiting time for all customers, every customer add their own waiting time to this variable, mutex_lock is necessary.
int queue_length[2];// variable stores the real-time queue length information; mutex_lock needed

int queue_status[2] ={0}; // variable to record the status of a queue, the value could be idle (not using by any clerk) or the clerk id (1 ~ 4), indicating that the corresponding clerk is now signaling this queue.
bool customer_selected[2] = {false}; // variable to record if the first customer in a queue has been successfully selected and left the queue.

struct Node* head = NULL;

// pthread_cond_t economy_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_cv[2] = {PTHREAD_COND_INITIALIZER};
// pthread_cond_t business_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t clerk_cv[4] = {PTHREAD_COND_INITIALIZER};
pthread_mutex_t queue_mutex[2];
// pthread_mutex_t economy_mutex;
// pthread_mutex_t business_mutex;
pthread_mutex_t clerk_mutex[4];
pthread_mutex_t time_mutex;

struct Queue* business_q = NULL;
struct Queue* economy_q = NULL;

int main(int argc, char* argv[]) {

	gettimeofday(&init_time, NULL);

	if(argc != 2){
		printf("Error: Incorrect usage.\nCorrect usage: ./ACS <filename>\n");
		exit(1);
	}

	int customer_amnt = 0;
	char* customer_file = argv[1];
	FILE *ptr;
	char line[256];
	int cnt = 0;
	if((ptr = fopen(customer_file, "r")) == NULL){
		printf("Error opening file: %s\n", customer_file);
		exit(1);
	}
	while(fgets(line, sizeof(line), ptr)){
		if(cnt == 0){
			customer_amnt = atoi(line);
			cnt++;
		}
		else{
			add_customer_details(line);
		}
	}

	business_q = createQueue(256);
	economy_q = createQueue(256);

	pthread_t clerkId[CLERK_AMNT];
	int clerk_info[CLERK_AMNT] = {1, 2, 3, 4};
	pthread_t customId[customer_amnt];
	int custom_info[customer_amnt];

	for(int i = 0; i < 2; i++){
		if(pthread_mutex_init(&queue_mutex[i], NULL) != 0){
			printf("Error: Queue mutex init failed\n");
			exit(1);
		}
	}

	for(int i = 0; i < 4; i++){
		if(pthread_mutex_init(&clerk_mutex[i], NULL) != 0){
			printf("Error: Clerk mutex init failed\n");
			exit(1);
		}
	}

	if(pthread_mutex_init(&time_mutex, NULL) != 0){
		printf("Error: Time mutex init failed\n");
		exit(1);
	}

	//create clerk thread (optional)
	for(int i = 0; i < CLERK_AMNT; i++){ // number of clerks
		pthread_create(&clerkId[i], NULL, clerk_entry, (void *)&clerk_info[i]); // clerk_info: passing the clerk information (e.g., clerk ID) to clerk thread
	}

	//create customer thread
	struct Node* tmp = head;
	for(int i = 0; i < customer_amnt; i++){ // number of customers
		custom_info[i] = i + 1;
		pthread_create(&customId[i], NULL, customer_entry, (void *)tmp); //custom_info: passing the customer information (e.g., customer ID, arrival time, service time, etc.) to customer thread
		tmp = tmp->next;
	}
	for(int i = 0; i < customer_amnt; i++){
		pthread_join(customId[i], NULL);
	}

	// pthread_mutex_destroy(&business_mutex);
	// pthread_mutex_destroy(&economy_mutex);
	// pthread_mutex_destroy(&clerk1_mutex);
	// pthread_mutex_destroy(&clerk2_mutex);
	// pthread_mutex_destroy(&clerk3_mutex);
	// pthread_mutex_destroy(&clerk4_mutex);
	// pthread_cond_destroy(&business_cv);
	// pthread_cond_destroy(&economy_cv);
	// pthread_cond_destroy(&clerk1_cv);
	// pthread_cond_destroy(&clerk2_cv);
	// pthread_cond_destroy(&clerk3_cv);
	// pthread_cond_destroy(&clerk4_cv);
	
	// calculate the average waiting time of all customers
	return 0;
}

double getCurrentSimulationTime(){
	struct timeval cur_time;
	double cur_secs, init_secs;

	pthread_mutex_lock(&time_mutex);
	init_secs = (init_time.tv_sec + (double) init_time.tv_usec / 1000000);
	pthread_mutex_unlock(&time_mutex);

	gettimeofday(&cur_time, NULL);
	cur_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);

	return cur_secs - init_secs;
}

void add_customer_details(char* str){
	int len = strlen(str);
	int user_id = 0;
	int class_type = 0;
	int arrival_time = 0;
	int service_time = 0;
	int type = 0;
	int cnt = 0;
	char temp[256];

	for(int i = 0; i < len; i++){
		if(str[i] == ':' || str[i] == ','){
			if(type == 0){
				user_id = atoi(temp);
			}
			else if(type == 1){
				class_type = atoi(temp);
			}
			else if(type == 2){
				arrival_time = atoi(temp);
			}
			cnt = 0;
			type++;
		}
		else{
			temp[cnt] = str[i];
			temp[cnt+1] = '\0';
			cnt++;
		}
	}
	service_time = atoi(temp);
	add(&head, user_id, class_type, service_time, arrival_time);
}

// function entry for customer threads

void * customer_entry(void * cus_info){
	struct Node* p_myInfo = (struct Node*) cus_info;

	// printf("arrival time %d\n", p_myInfo->arrival_time);
	int arrival_time = p_myInfo->arrival_time;
	int user_id = p_myInfo->user_id;
	int selected_queue = p_myInfo->class_type;
	int service_time = p_myInfo->service_time;
	struct Queue* nodes_queue = NULL;

	usleep(arrival_time * 100000);

	fprintf(stdout, "A customer arrives: customer ID %2d. \n", user_id);

	pthread_mutex_lock(&queue_mutex[selected_queue]);

	fprintf(stdout, "A customer enters a queue: the queue ID %1d, and length of the queue %2d.\n", selected_queue, queue_length[selected_queue]);

	if(selected_queue == 0){
		nodes_queue = economy_q;
		// enqueue(economy_q, user_id);
	}
	else{
		nodes_queue = business_q;
		// enqueue(business_q, user_id);
	}
	enqueue(nodes_queue, user_id);
	double queue_enter_time = getCurrentSimulationTime();
	queue_length[selected_queue]++;

	while(1){
		// printf("test\n");
			// printf("Custome %d wait\n", user_id);
		pthread_cond_wait(&queue_cv[selected_queue], &queue_mutex[selected_queue]);

		// printf("test\n");
		if(user_id == front(nodes_queue) && !customer_selected[selected_queue]){
			dequeue(nodes_queue);
			queue_length[selected_queue]--;
			customer_selected[selected_queue] = true;
			break;
		}		
	}

	pthread_mutex_unlock(&queue_mutex[selected_queue]);

	usleep(10);

	int clerk_woke_me_up = queue_status[selected_queue];
	queue_status[selected_queue] = 0;

	double curr_time = getCurrentSimulationTime();
	fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d.\n", queue_enter_time, user_id, clerk_woke_me_up);

	usleep(service_time * 100000);

	curr_time = getCurrentSimulationTime();
	fprintf(stdout, "A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d.\n", curr_time, user_id, clerk_woke_me_up);

	pthread_cond_signal(&clerk_cv[clerk_woke_me_up - 1]);

	pthread_exit(NULL);
	
	return NULL;
}

// function entry for clerk threads
void *clerk_entry(void * clerkNum){
	int* clerkID = (int *) clerkNum;
	printf("Clerk ID %d\n", *clerkID);

	while(1){
		int selected_queue_ID = 0;

		// usleep(10);
		if(isEmpty(business_q) && !isEmpty(economy_q)){
			// printf("Locking economy in clerk\n");
			pthread_mutex_lock(&queue_mutex[0]);
		}
		else if(!isEmpty(business_q)){
			// printf("Locking business in clerk\n");		
			selected_queue_ID = 1;
			pthread_mutex_lock(&queue_mutex[1]);
		}
		else
			continue;


			// printf("Lacdsdvsd\n");	

		if(queue_status[selected_queue_ID] == 0)
			queue_status[selected_queue_ID] = *clerkID;
		else
			continue;

		pthread_cond_broadcast(&queue_cv[selected_queue_ID]);

		customer_selected[selected_queue_ID] = false;

		pthread_mutex_unlock(&queue_mutex[selected_queue_ID]);

		pthread_mutex_lock(&clerk_mutex[*clerkID - 1]);

		pthread_cond_wait(&clerk_cv[*clerkID - 1], &clerk_mutex[*clerkID - 1]);

		pthread_mutex_unlock(&clerk_mutex[*clerkID - 1]);
	}

	pthread_exit(NULL);
	
	return NULL;
}
