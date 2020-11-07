#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#include "llist.h"
#include "queue.h"

#define CLERK_AMNT 4

// Function declarations
void add_customer_details(char* str);
void * customer_entry(void * cus_info);
void *clerk_entry(void * clerk_ID);

// Global Variables 
static struct timeval init_time;
double business_waiting_time = 0.0;
int business_total = 0;
int economy_total = 0;
double economy_waiting_time = 0.0;
int queue_length[2] = {0, 0};
int queue_status[2] ={0, 0};
bool customer_selected[2] = {false};
struct Node* head = NULL;

// Mutexes and conditional variables
pthread_cond_t queue_cv[2] = {PTHREAD_COND_INITIALIZER};
pthread_cond_t clerk_cv[4] = {PTHREAD_COND_INITIALIZER};
pthread_mutex_t queue_mutex[2];
pthread_mutex_t clerk_mutex[4];
pthread_mutex_t time_mutex;

// Queues
struct Queue* business_queue = NULL;
struct Queue* economy_queue = NULL;

int main(int argc, char* argv[]) {

	gettimeofday(&init_time, NULL);

	if(argc != 2){
		printf("Error: Incorrect usage.\nCorrect usage: ./ACS <filename>\n");
		exit(1);
	}

	FILE *ptr;
	int customer_amnt = 0;
	char* customer_file = argv[1];
	char line[256];
	int cnt = 0;

	// Parse file for customer information
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

	// Initialize queues
	business_queue = createQueue(customer_amnt);
	economy_queue = createQueue(customer_amnt);

	// Initialize mutexes
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

	// Create clerk threads
	pthread_t clerk[CLERK_AMNT];
	int clerk_info[CLERK_AMNT] = {1, 2, 3, 4};
	for(int i = 0; i < CLERK_AMNT; i++){
		pthread_create(&clerk[i], NULL, clerk_entry, (void *)&clerk_info[i]);
	}

	// Create customer threads
	pthread_t customId[customer_amnt];
	int custom_info[customer_amnt];
	struct Node* tmp = head;
	for(int i = 0; i < customer_amnt; i++){
		custom_info[i] = i + 1;
		pthread_create(&customId[i], NULL, customer_entry, (void *)tmp);
		tmp = tmp->next;
	}

	for(int i = 0; i < customer_amnt; i++){
		pthread_join(customId[i], NULL);
	}

	// Destroy mutexes and conditional variables
	for(int i = 0; i < 2; i++){
		pthread_mutex_destroy(&queue_mutex[i]);
		pthread_cond_destroy(&queue_cv[i]);	
	}
	for(int i = 0; i < CLERK_AMNT; i++){
		pthread_mutex_destroy(&clerk_mutex[i]);
		pthread_cond_destroy(&clerk_cv[i]);
	}
	pthread_mutex_destroy(&time_mutex);

	// Find and print the average waiting times
	double total_waiting_time = business_waiting_time + economy_waiting_time;
	double total_avg = total_waiting_time / customer_amnt;
	double business_avg = business_waiting_time / business_total;
	double economy_avg = economy_waiting_time / economy_total;

	printf("The average waiting time for all customers in the system is: %.2f seconds. \n", total_avg);
	printf("The average waiting time for all business-class customers is: %.2f seconds. \n", business_avg);
	printf("The average waiting time for all economy-class customers is: %.2f seconds. \n", economy_avg);

	return 0;
}

/*
 * Function: get_current_time
 *
 * Gets the current simulation time of the program.
 */
double get_current_time(){
	struct timeval cur_time;
	double cur_secs, init_secs;

	pthread_mutex_lock(&time_mutex);
	init_secs = (init_time.tv_sec + (double) init_time.tv_usec / 1000000);
	pthread_mutex_unlock(&time_mutex);

	gettimeofday(&cur_time, NULL);
	cur_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);

	return cur_secs - init_secs;
}

/*
 * Function: add_customer_details
 * Parameter(s):
 * str, a string of customer information from the input file.
 *
 * Parses a string of customer information which is fetched from the input file.
 * Follows the formatting specified in the assignment description to assign appropriate data to specified variables.
 * As the loop moves through the string, it stores the integer it is tracing into a string. Once a separator character is found,
 * the string being stored is turned into an int and the temp string is reset to parse the next #.
 * Adds these variables to a node in a linked list dedicated to customers.
 */
void add_customer_details(char* str){
	int len = strlen(str);
	int user_id = 0;
	int class_type = 0;
	int arrival_time = 0;
	int service_time = 0;
	int type = 0;
	int cnt = 0;
	char temp[256];

	// Iterate through the string, char by char
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

/*
 * Function: customer_entry
 * Parameter(s):
 * cus_info, a node storing information regarding the customer
 *
 * Simulates a customer's arrival time, enqueuing into it's expected queue, waiting for an available clerk to serve it,
 * and service time.
 * Calculates and outputs this information to stdout.
 */
void * customer_entry(void * cus_info){
	struct Node* curr_info = (struct Node*) cus_info;

	int arrival_time = curr_info->arrival_time;
	int user_id = curr_info->user_id;
	int selected_queue_ID = curr_info->class_type;
	int service_time = curr_info->service_time;
	struct Queue* nodes_queue = NULL;

	// Customer arrives
	usleep(arrival_time * 100000);
	fprintf(stdout, "A customer arrives: customer ID %2d. \n", user_id);

	// -------------------------QUEUE MUTEX LOCKED-------------------------//
	pthread_mutex_lock(&queue_mutex[selected_queue_ID]);

	fprintf(stdout, "A customer enters a queue: the queue ID %1d, and length of the queue %2d.\n", selected_queue_ID, queue_length[selected_queue_ID]);

	// Enqueue to appropriate queue
	if(selected_queue_ID == 0){
		enqueue(economy_queue, user_id);
	}
	else{
		enqueue(business_queue, user_id);
	}
	double queue_entry_time = get_current_time();
	queue_length[selected_queue_ID]++;

	while(1){
		// Wait for a clerk to signal the customer
		pthread_cond_wait(&queue_cv[selected_queue_ID], &queue_mutex[selected_queue_ID]);

		// If the customer is at the front of the queue and that queue has selected a customer
		if((user_id == front(economy_queue) || user_id == front(business_queue)) && !customer_selected[selected_queue_ID]){
			// Deuque from appropriate queue
			if(selected_queue_ID == 0){
				dequeue(economy_queue);
			}
			else{
				dequeue(business_queue);
			}
			queue_length[selected_queue_ID]--;
			customer_selected[selected_queue_ID] = true;
			break;
		}		
	}

	pthread_mutex_unlock(&queue_mutex[selected_queue_ID]);
	// -------------------------QUEUE MUTEX UNLOCKED-------------------------//

	usleep(10);

	// -------------------------QUEUE MUTEX LOCKED-------------------------//
	pthread_mutex_lock(&queue_mutex[selected_queue_ID]);

	// Determine which clerk selected the customer, reset queue status to idle
	int selected_clerk = queue_status[selected_queue_ID];
	queue_status[selected_queue_ID] = 0;

	pthread_mutex_unlock(&queue_mutex[selected_queue_ID]);
	// -------------------------QUEUE MUTEX UNLOCKED-------------------------//

	double start_service = get_current_time();
	fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d.\n", start_service, user_id, selected_clerk);

	// A customer is serviced
	usleep(service_time * 100000);

	double finish_service = get_current_time();
	fprintf(stdout, "A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d.\n", finish_service, user_id, selected_clerk);

	// -------------------------TIME MUTEX LOCKED-------------------------//
	pthread_mutex_lock(&time_mutex);
	// Add waiting times (time customer is waiting in queue) to totals
	if(selected_queue_ID == 0){
		economy_waiting_time += (start_service - queue_entry_time);
		economy_total++;
	}
	else{
		business_waiting_time += (start_service - queue_entry_time);
		business_total++;
	}
	pthread_mutex_unlock(&time_mutex);
	// -------------------------TIME MUTEX UNLOCKED-------------------------//

	// Signal to clerk that customer is done service
	pthread_cond_signal(&clerk_cv[selected_clerk - 1]);

	pthread_exit(NULL);
	
	return NULL;
}

/*
 * Function: clerk_entry
 * Parameter(s):
 * clerk, a node storing information regarding the customer
 *
 * Simulates a customer's arrival time, enqueuing into it's expected queue, waiting for an available clerk to serve it,
 * and service time.
 * Calculates and outputs this information to stdout.
 */
void *clerk_entry(void * clerk_ID){
	int* clerk = (int *) clerk_ID;
	int selected_queue_ID = 0;

	while(1){
		// Wait for a customer to enter any queue, prioritize business
		if(queue_length[1] != 0){
			selected_queue_ID = 1;
		}
		else if(queue_length[0] != 0)
			selected_queue_ID = 0;
		else
			continue;

		// -------------------------QUEUE MUTEX LOCKED-------------------------//
		pthread_mutex_lock(&queue_mutex[selected_queue_ID]);

		// If the selected queue is being served, continue waiting. Else continue function
		if(queue_status[selected_queue_ID] != 0){
			pthread_mutex_unlock(&queue_mutex[selected_queue_ID]);
			usleep(10);
			continue;
		}
		else{
			queue_status[selected_queue_ID] = *clerk;			
			pthread_mutex_unlock(&queue_mutex[selected_queue_ID]);
		}
		// -------------------------QUEUE MUTEX UNLOCKED-------------------------//


		// -------------------------QUEUE MUTEX LOCKED-------------------------//
		pthread_mutex_lock(&queue_mutex[selected_queue_ID]);

		// Broadcast to the customer that it is time to service it
		customer_selected[selected_queue_ID] = false;
		pthread_cond_broadcast(&queue_cv[selected_queue_ID]);

		pthread_mutex_unlock(&queue_mutex[selected_queue_ID]);
		// -------------------------QUEUE MUTEX UNLOCKED-------------------------//


		// -------------------------CLERK MUTEX LOCKED-------------------------//
		pthread_mutex_lock(&clerk_mutex[*clerk - 1]);

		// Wait for the customer to finish it's service
		pthread_cond_wait(&clerk_cv[*clerk - 1], &clerk_mutex[*clerk - 1]);

		pthread_mutex_unlock(&clerk_mutex[*clerk - 1]);
		// -------------------------CLERK MUTEX UNLOCKED-------------------------//
	}

	pthread_exit(NULL);
	
	return NULL;
}
