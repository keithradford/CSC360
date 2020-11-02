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
pthread_mutex_t clerk1_mutex;
pthread_mutex_t clerk2_mutex;
pthread_mutex_t clerk3_mutex;
pthread_mutex_t clerk4_mutex;


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

	business_q = createQueue(256);
	economy_q = createQueue(256);

	pthread_t clerkId[CLERK_AMNT];
	int clerk_info[CLERK_AMNT] = {1, 2, 3, 4};
	pthread_t customId[customer_amnt];
	int custom_info[customer_amnt];

	if(pthread_mutex_init(&economy_mutex, NULL) != 0 || pthread_mutex_init(&business_mutex, NULL) != 0){
		printf("Error: Queue mutex init failed\n");
		exit(1);
	}

	if(pthread_mutex_init(&clerk1_mutex, NULL) != 0 || pthread_mutex_init(&clerk2_mutex, NULL) != 0 || pthread_mutex_init(&clerk3_mutex, NULL) != 0 || pthread_mutex_init(&clerk4_mutex, NULL) != 0){
		printf("Error: Clerk mutex init failed\n");
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

	pthread_mutex_destroy(&business_mutex);
	pthread_mutex_destroy(&economy_mutex);
	pthread_cond_destroy(&business_cv);
	pthread_cond_destroy(&economy_cv);
	pthread_cond_destroy(&clerk1_cv);
	pthread_cond_destroy(&clerk2_cv);
	pthread_cond_destroy(&clerk3_cv);
	pthread_cond_destroy(&clerk4_cv);
	
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
			cnt++;
		}
	}
	service_time = atoi(temp);
	add(&head, user_id, class_type, service_time, arrival_time);
}

// function entry for customer threads

void * customer_entry(void * cus_info){
	struct Node* p_myInfo = (struct Node*) cus_info;

	usleep((p_myInfo->arrival_time)/100000);
	
	fprintf(stdout, "A customer arrives: customer ID %2d. \n", p_myInfo->user_id);

	struct Queue* selected_queue = NULL;
	double queue_enter_time = 0.0;
	
	/* Enqueue operation: get into either business queue or economy queue by using p_myInfo->class_type*/	
	if(p_myInfo->class_type == 0){
		selected_queue = economy_q;
		pthread_mutex_lock(&economy_mutex); 
	}
	else{
		selected_queue = business_q;
		pthread_mutex_lock(&business_mutex); 
	}
	
	{
		fprintf(stdout, "A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", p_myInfo->class_type, selected_queue->size);

		if(p_myInfo->class_type == 0){
			enqueue(economy_q, p_myInfo->user_id);
		}
		else{
			enqueue(business_q, p_myInfo->user_id);
		}


		// toString(selected_queue);

		queue_enter_time = gettimeofday(&init_time, NULL);
		
		while (true) {
			if(p_myInfo->class_type == 0){
				pthread_cond_wait(&economy_cv, &economy_mutex);
			}
			else{
				pthread_cond_wait(&business_cv, &business_mutex);
			}
			if ((p_myInfo->user_id == front(selected_queue)) && !(customer_selected[p_myInfo->class_type])){
				dequeue(selected_queue);
				customer_selected[p_myInfo->class_type] = true; // update the customer_selected variable to indicate that the first customer has been selected from the queue
				break;
			}
		}
			
	}

	// pthread_mutex_unlock(&selected); //unlock mutex_lock such that other customers can enter into the queue
	
	/* Try to figure out which clerk awoken me, because you need to print the clerk Id information */
	usleep(10); // Add a usleep here to make sure that all the other waiting threads have already got back to call pthread_cond_wait. 10 us will not harm your simulation time.
	int clerk_woke_me_up = queue_status[p_myInfo->class_type];
	queue_status[p_myInfo->class_type] = 0;

	if(p_myInfo->class_type == 0){
		pthread_mutex_unlock(&economy_mutex);
	}
	else{
		pthread_mutex_unlock(&business_mutex);
	}
	
	// get the current machine time; updates the overall_waiting_time
	double cur_time = gettimeofday(&init_time, NULL);
	
	fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n", queue_enter_time, p_myInfo->user_id, clerk_woke_me_up);
	
	usleep((p_myInfo->service_time)/100000);
	
	/* get the current machine time; */
	cur_time = gettimeofday(&init_time, NULL);
	fprintf(stdout, "A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", cur_time, p_myInfo->user_id, clerk_woke_me_up);
	
	// pthread_cond_signal( convar of the clerk signaled me ); // Notify the clerk that service is finished, it can serve another customer
	if(clerk_woke_me_up == 1)
		pthread_cond_signal(&clerk1_cv); // wait for the customer to finish its service, clerk busy
	else if(clerk_woke_me_up == 2)
		pthread_cond_signal(&clerk2_cv);
	else if(clerk_woke_me_up == 3)
		pthread_cond_signal(&clerk3_cv);
	else if(clerk_woke_me_up == 4)
		pthread_cond_signal(&clerk4_cv);

	pthread_exit(NULL);
	
	return NULL;
}

// function entry for clerk threads
void *clerk_entry(void * clerkNum){
	
	while(true){
		// clerk is idle now
		int selected_queue_ID = 0;

		int* clerkID = (int *) clerkNum;

		if(*clerkID == 1)
			pthread_mutex_lock(&clerk1_mutex);
		else if(*clerkID == 2)
			pthread_mutex_lock(&clerk2_mutex);
		else if(*clerkID == 3)
			pthread_mutex_lock(&clerk3_mutex);
		else
			pthread_mutex_lock(&clerk4_mutex);
		
		/* selected_queue_ID = Select the queue based on the priority and current customers number */
		if(isEmpty(business_q) && !isEmpty(economy_q)){
			// pthread_mutex_lock(&economy_mutex);			
			queue_status[selected_queue_ID] = *clerkID; // The current clerk (clerkID) is signaling this queue			
			pthread_cond_broadcast(&economy_cv); // Awake the customer (the one enter into the queue first) from the longest queue (notice the customer he can get served now)
			customer_selected[selected_queue_ID] = false; // set the initial value as the customer has not selected from the queue.

			if(queue_status[selected_queue_ID] == 0){
				if(*clerkID == 1)
					pthread_cond_wait(&clerk1_cv, &economy_mutex); // wait for the customer to finish its service, clerk busy
				else if(*clerkID == 2)
					pthread_cond_wait(&clerk2_cv, &economy_mutex);
				else if(*clerkID == 3)
					pthread_cond_wait(&clerk3_cv, &economy_mutex);
				else if(*clerkID == 4)
					pthread_cond_wait(&clerk4_cv, &economy_mutex);
			}

			// pthread_mutex_unlock(&economy_mutex);
		}
		else if(!isEmpty(business_q)){
			selected_queue_ID = 1;
			// pthread_mutex_lock(&business_mutex);			
			queue_status[selected_queue_ID] = *clerkID; // The current clerk (clerkID) is signaling this queue			
			pthread_cond_broadcast(&business_cv); // Awake the customer (the one enter into the queue first) from the longest queue (notice the customer he can get served now)
			customer_selected[selected_queue_ID] = false; // set the initial value as the customer has not selected from the queue.
			
			if(queue_status[selected_queue_ID] == 0){
				if(*clerkID == 1)
					pthread_cond_wait(&clerk1_cv, &business_mutex); // wait for the customer to finish its service, clerk busy
				else if(*clerkID == 2)
					pthread_cond_wait(&clerk2_cv, &business_mutex);
				else if(*clerkID == 3)
					pthread_cond_wait(&clerk3_cv, &business_mutex);
				else if(*clerkID == 4)
					pthread_cond_wait(&clerk4_cv, &business_mutex);
			}

			// pthread_mutex_unlock(&business_mutex);
		}

		if(*clerkID == 1)
			pthread_mutex_unlock(&clerk1_mutex);
		else if(*clerkID == 2)
			pthread_mutex_unlock(&clerk2_mutex);
		else if(*clerkID == 3)
			pthread_mutex_unlock(&clerk3_mutex);
		else
			pthread_mutex_unlock(&clerk4_mutex);

	}

	pthread_exit(NULL);
	
	return NULL;
}
