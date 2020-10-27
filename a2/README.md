# CSC 360 Assignment 2

##Compiling Instructions

1. Navigate to the directory for the program
2. Type "make ACS" in the command line to compile
3. Type "./ACS <inputfile>" to run

##Design Document

1. n threads for customer (n = # of customers), 4 threads for the 4 clerks, therefore n + 4 threads total
2. Threads work independently. As long as each thread is following the same logic, there shouldn't be a need for the threads to work dependently.
3. Two mutexes. One for each queue (business and economy). The mutex will guard the deQueue operations so no customer can be dequeued while a customer is being serviced from that queue.
4. Main thread will be idle.
5. As nodes in a linked list. Each node will contain the relevent information for each customer (class, service time, etc.).
6. The main linked list containing the customer information will not need to be modified after every customer has been initialzied in it. Furthermore, queues should not be modified concurrently due to the mutex lock guarding deQueue for each queue.
7. Two convars, one for each queue. They will have the same properties.
	(a) Is there an available clerk?
	(b) The two existing mutex.
	(c) deQueue
8. 
	for customers in input.txt 
		add to linked list

	for number of clerks
		create threads for clerks
		select queue of highest priority and signal to it
		unlock mutex
		awake customer
		lock mutex

	for number of customers
		create threads for customers
		wait for arrival time
		enqueue
		while convar signalling available clerk not true
			if signalled
				deQueue
				break


