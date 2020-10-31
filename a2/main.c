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
 
struct timeval init_time; // use this variable to record the simulation start time; No need to use mutex_lock when reading this variable since the value would not be changed by thread once the initial time was set.
double overall_waiting_time; //A global variable to add up the overall waiting time for all customers, every customer add their own waiting time to this variable, mutex_lock is necessary.
int queue_length[2];// variable stores the real-time queue length information; mutex_lock needed

int queue_status[2] ={0}; // variable to record the status of a queue, the value could be idle (not using by any clerk) or the clerk id (1 ~ 4), indicating that the corresponding clerk is now signaling this queue.
bool customer_selected[2] = {false}; // variable to record if the first customer in a queue has been successfully selected and left the queue.

struct Node* head = NULL;

pthread_cond_t economy_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t business_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t clerk1_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t clerk2_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t clerk3_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t clerk4_cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t economy_mutex;
pthread_mutex_t business_mutex;

struct Queue* business_q = NULL;
struct Queue* economy_q = NULL;

int main(int argc, char* argv[]) {

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

	business_q = createQueue(customer_amnt);
	economy_q = createQueue(customer_amnt);

	pthread_t clerkId[CLERK_AMNT];
	int clerk_info[CLERK_AMNT] = {1, 2, 3, 4};
	pthread_t customId[customer_amnt];
	int custom_info[customer_amnt];

	if(pthread_mutex_init(&economy_mutex, NULL) != 0 || pthread_mutex_init(&business_mutex, NULL) != 0){
		printf("Error: Mutex init failed\n");
		exit(1);
	}

	//create clerk thread (optional)
	for(int i = 0; i < CLERK_AMNT; i++){ // number of clerks
		pthread_create(&clerkId[i], NULL, clerk_entry, (void *)&clerk_info[i]); // clerk_info: passing the clerk information (e.g., clerk ID) to clerk thread
	}
	
	//create customer thread
	for(int i = 0; i < customer_amnt; i++){ // number of customers
		custom_info[i] = i + 1;
		pthread_create(&customId[i], NULL, customer_entry, (void *)&custom_info[i]); //custom_info: passing the customer information (e.g., customer ID, arrival time, service time, etc.) to customer thread
	}
	// wait for all customer threads to terminate
	// forEach customer thread{
	// 	pthread_join(...);
	// }
	// destroy mutex & condition variable (optional)
	
	// calculate the average waiting time of all customers
	return 0;
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
			// printf("string %s\n", temp);
			cnt++;
		}
	}
	service_time = atoi(temp);
	add(&head, user_id, class_type, service_time, arrival_time);
}

// function entry for customer threads

void * customer_entry(void * cus_info){
	
	int *user_id = (int *) cus_info;

	struct Node* p_myInfo = getNode(&head, *user_id);
	
	usleep(p_myInfo->arrival_time);
	
	fprintf(stdout, "A customer arrives: customer ID %2d. \n", p_myInfo->user_id);

	pthread_mutex_t selected;

	struct Queue* selected_queue = NULL;

	pthread_cond_t selected_cv;

	float queue_enter_time = 0.0;
	
	/* Enqueue operation: get into either business queue or economy queue by using p_myInfo->class_type*/	
	if(p_myInfo->class_type == 0){
		selected_cv = economy_cv;
		selected_queue = economy_q;
		selected = economy_mutex;
		}
	else{
		selected_cv = business_cv;
		selected_queue = business_q;
		selected = business_mutex;
	}
	
	// 2, 3
	pthread_mutex_lock(&selected); 
	{
		fprintf(stdout, "A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", p_myInfo->class_type, selected_queue->size);

		if(p_myInfo->class_type == 0){
			enqueue(economy_q, p_myInfo->user_id);
		}
		else{
			enqueue(business_q, p_myInfo->user_id);
		}

		queue_enter_time = gettimeofday(&init_time, NULL);

		// selected_queue->size = selected_queue->size + 1;
		
		while (true) {
			pthread_cond_wait(&selected_cv, &selected);
			if ((p_myInfo->user_id == selected_queue->array[selected_queue->front]) && !(customer_selected[p_myInfo->class_type])){
				int tmp = dequeue(selected_queue);
				customer_selected[p_myInfo->class_type] = true; // update the customer_selected variable to indicate that the first customer has been selected from the queue
				break;
			}
		}
			
	}
	pthread_mutex_unlock(&selected); //unlock mutex_lock such that other customers can enter into the queue
	
	/* Try to figure out which clerk awoken me, because you need to print the clerk Id information */
	usleep(10); // Add a usleep here to make sure that all the other waiting threads have already got back to call pthread_cond_wait. 10 us will not harm your simulation time.
	int clerk_woke_me_up = queue_status[p_myInfo->class_type];
	queue_status[p_myInfo->class_type] = 0;
	
	// get the current machine time; updates the overall_waiting_time
	float cur_time = gettimeofday(&init_time, NULL);
	
	fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n", queue_enter_time, p_myInfo->user_id, clerk_woke_me_up);
	
	usleep((p_myInfo->service_time)/100000);
	
	/* get the current machine time; */
	cur_time = gettimeofday(&init_time, NULL);
	fprintf(stdout, "A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", cur_time, p_myInfo->user_id, clerk_woke_me_up);
	
	// pthread_cond_signal(/* convar of the clerk signaled me */); // Notify the clerk that service is finished, it can serve another customer
	
	pthread_exit(NULL);
	
	return NULL;
}

// function entry for clerk threads
void *clerk_entry(void * clerkNum){
	
	while(true){
		// clerk is idle now

		pthread_mutex_t selected;
		struct Queue* selected_queue = NULL;
		pthread_cond_t selected_cv;
		int selected_queue_ID = 0;
		
		/* selected_queue_ID = Select the queue based on the priority and current customers number */
		if(isEmpty(business_q)){
			selected = economy_mutex;
			selected_queue = economy_q;
			selected_cv = economy_cv;
		}
		else{
			selected = business_mutex;
			selected_queue = business_q;
			selected_cv = business_cv;
			selected_queue_ID = 1;
		}

		
		pthread_mutex_lock(&selected);

		int* clerkID = (int *) clerkNum;
		
		queue_status[selected_queue_ID] = *clerkID; // The current clerk (clerkID) is signaling this queue
		
		pthread_cond_broadcast(&selected_cv); // Awake the customer (the one enter into the queue first) from the longest queue (notice the customer he can get served now)
		
		customer_selected[selected_queue_ID] = false; // set the initial value as the customer has not selected from the queue.
		
		pthread_mutex_unlock(&selected);
		
		if(*clerkID == 1)
			pthread_cond_wait(&clerk1_cv, &selected); // wait for the customer to finish its service, clerk busy
		else if(*clerkID == 2)
			pthread_cond_wait(&clerk2_cv, &selected);
		else if(*clerkID == 3)
			pthread_cond_wait(&clerk3_cv, &selected);
		else
			pthread_cond_wait(&clerk4_cv, &selected);
	}
	
	pthread_exit(NULL);
	
	return NULL;
}
