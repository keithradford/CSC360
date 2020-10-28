#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "llist.h"

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

int queue_status[2]; // variable to record the status of a queue, the value could be idle (not using by any clerk) or the clerk id (1 ~ 4), indicating that the corresponding clerk is now signaling this queue.
bool customer_selected[2] = {false}; // variable to record if the first customer in a queue has been successfully selected and left the queue.

struct Node* head = NULL;
/* Other global variable may include: 
 1. condition_variables (and the corresponding mutex_lock) to represent each queue; 
 2. condition_variables to represent clerks
 3. others.. depend on your design
 */

int main(int argc, char* argv[]) {

	// initialize all the condition variable and thread lock will be used
	pthread_cond_t economy_cv;
	pthread_cond_t business_cv;
	
	/** Read customer information from txt file and store them in the structure you created 
		
		1. Allocate memory(array, link list etc.) to store the customer information.
		2. File operation: fopen fread getline/gets/fread ..., store information in data structure you created

	*/
	if(argc > 2 || argc < 2){
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



	//create clerk thread (optional)
	// for(int i = 0, i < CLERK_AMNT; i++){ // number of clerks
	// 	pthread_create(&clerkId[i], NULL, clerk_entry, (void *)&clerk_info[i]); // clerk_info: passing the clerk information (e.g., clerk ID) to clerk thread
	// }
	
	// //create customer thread
	// for(int i = 0, i < customer_amnt; i++){ // number of customers
	// 	pthread_create(&customId[i], NULL, customer_entry, (void *)&custom_info[i]); //custom_info: passing the customer information (e.g., customer ID, arrival time, service time, etc.) to customer thread
	// }
	// // wait for all customer threads to terminate
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
			printf("string %s\n", temp);
			cnt++;
		}
	}
	service_time = atoi(temp);
	add(&head, user_id, class_type, service_time, arrival_time);
}

// function entry for customer threads

// void * customer_entry(void * cus_info){
	
// 	struct customer_info * p_myInfo = (struct info_node *) cus_info;
	
// 	usleep(/* the arrival time of this customer */);
	
// 	fprintf(stdout, "A customer arrives: customer ID %2d. \n", p_myInfo->user_id);
	
// 	/* Enqueue operation: get into either business queue or economy queue by using p_myInfo->class_type*/
	
// 	// 2, 3
// 	pthread_mutex_lock(/* mutexLock of selected queue */); 
// 	{
// 		fprintf(stdout, "A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", /*...*/);

// 		enQueue();
// 		queue_enter_time = getCurSystemTime();
// 		queue_length[cur_queue]++;
		
// 		while (TRUE) {
// 			pthread_cond_wait(/* cond_var of selected queue */, /* mutexLock of selected queue */);
// 			if (I_am_Head_of_the_Queue && !customer_selected[cur_queue]) {
// 				deQueue();
// 				queue_length[cur_queue]--;
// 				customer_selected[cur_queue] = TRUE; // update the customer_selected variable to indicate that the first customer has been selected from the queue
// 				break;
// 			}
// 		}
			
// 	}
// 	pthread_mutex_unlock(/*mutexLock of selected queue*/); //unlock mutex_lock such that other customers can enter into the queue
	
// 	/* Try to figure out which clerk awoken me, because you need to print the clerk Id information */
// 	usleep(10); // Add a usleep here to make sure that all the other waiting threads have already got back to call pthread_cond_wait. 10 us will not harm your simulation time.
// 	clerk_woke_me_up = queue_status[cur_queue];
// 	queue_status[cur_queue] = IDLE;
	
// 	 get the current machine time; updates the overall_waiting_time
	
// 	fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n", /*...*/);
	
// 	usleep(/* as long as the service time of this customer */);
	
// 	/* get the current machine time; */
// 	fprintf(stdout, "A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", /* ... */);\
	
// 	pthread_cond_signal(/* convar of the clerk signaled me */); // Notify the clerk that service is finished, it can serve another customer
	
// 	pthread_exit(NULL);
	
// 	return NULL;
// }

// // function entry for clerk threads
// void *clerk_entry(void * clerkNum){
	
// 	while(TRUE){
// 		// clerk is idle now
		
// 		/* selected_queue_ID = Select the queue based on the priority and current customers number */
		
// 		pthread_mutex_lock(/* mutexLock of the selected queue */);
		
// 		queue_status[selected_queue_ID] = clerkID; // The current clerk (clerkID) is signaling this queue
		
// 		pthread_cond_broadcast(/* cond_var of the selected queue */); // Awake the customer (the one enter into the queue first) from the longest queue (notice the customer he can get served now)
		
// 		customer_selected[selected_queue_ID] = FALSE; // set the initial value as the customer has not selected from the queue.
		
// 		pthread_mutex_unlock(/* mutexLock of the selected queue */);
		
// 		pthread_cond_wait(/* convar of the current clerk */); // wait for the customer to finish its service, clerk busy
// 	}
	
// 	pthread_exit(NULL);
	
// 	return NULL;
// }
