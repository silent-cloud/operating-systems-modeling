/**
 * Author: John Lorenz Salva
 * 
 * buffer.c replicates the solution to the
 * bounded buffer problem using mutex locks
 * and counting semaphores. The consumption
 * order is "first in, first out". The production
 * order is "circular".
 **/

#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "buffer.h"
#define MAX_THREADS 93

buffer_item buffer[BUFFER_SIZE];
pthread_mutex_t mutex;
static int p_buf_index;
static int c_buf_index;
sem_t empty;
sem_t full;

void insert_item(buffer_item item, void *param);
void remove_item(buffer_item item, void *param);
void *consumer(void *param);
void *producer(void *param);

int main(int argc, char *argv[]) {
	
	int consumers = 0;
	int producers = 0;
	int sleep_time = 0;
	int p_index;
	int c_index;
	
	//Command Line Arguments
	if (argc != 4) {
		printf("Usage: buffer <sleep time> <# of producer threads> <# of consumer threads>\n");
		return EXIT_FAILURE;
	} else {
		int i;
		char* sleepArg = argv[1];
		char* prodArg = argv[2];
		char* consArg = argv[3];
		
		//Check if sleep time argument is a number
		for (i = 0; sleepArg[i] != '\0'; i++) {
			if (!isdigit(sleepArg[i])) {
				printf("Usage: buffer <sleep time> <# of producer threads> <# of consumer threads>\n");
				return EXIT_FAILURE;
			}
		}
		
		//Check if amount of producer threads argument is a number
		for (i = 0; prodArg[i] != '\0'; i++) {
			if (!isdigit(prodArg[i])) {
				printf("Usage: buffer <sleep time> <# of producer threads> <# of consumer threads>\n");
				return EXIT_FAILURE;
			}
		}
		
		//Check if amount of consumer threads argument is a number
		for (i = 0; consArg[i] != '\0'; i++) {
			if (!isdigit(consArg[i])) {
				printf("Usage: buffer <sleep time> <# of producer threads> <# of consumer threads>\n");
				return EXIT_FAILURE;
			}
		}
		
		//Convert args to integers
		sleep_time = atoi(argv[1]);
		producers = atoi(argv[2]);
		consumers = atoi(argv[3]);
		
		//Check if the amount of threads being created exceed the max amount that can be allowed
		if (producers + consumers > MAX_THREADS) {
			printf("Exceeded max limit of threads (%d threads) for this linux machine.\n", MAX_THREADS);
			sleep(1);
			printf("Use a smaller number of consumer and producer threads.\n");
			sleep(1);
			return EXIT_FAILURE;
		}
	}
		
	
	//Initialize values
	srand(time(NULL));
	pthread_mutex_init(&mutex, NULL);
	sem_init(&empty, 0, BUFFER_SIZE);
	sem_init(&full, 0, 0);
	int count = 0;
	c_buf_index = 0;
	p_buf_index = 0;
	c_index = 0;
	p_index = 0;
	
	//Create producer thread(s)
	for (count = 0; count < producers; count++) {
		p_index++;
		pthread_t p_tid;
		pthread_attr_t p_attr;
		pthread_attr_init(&p_attr);
		pthread_create(&p_tid, &p_attr, producer, (void*) p_index);
	}
	
	//Create consumer thread(s)
	for (count = 0; count < consumers; count++) {
		c_index++;
		pthread_t c_tid;
		pthread_attr_t c_attr;
		pthread_attr_init(&c_attr);
		pthread_create(&c_tid, &c_attr, consumer, (void*) c_index);
	}
	
	//Sleep
	sleep(sleep_time);
	
	//Exit
	return EXIT_SUCCESS;
}

/**
 * Inserts an item into the buffer with the protection of 
 * semaphore and mutex
 *
 * @param item 	Random number to be inserted into the buffer
 * @param param	Locally generated thread ID 
 */

void insert_item(buffer_item item, void *param) {
   sem_wait(&empty);
   pthread_mutex_lock(&mutex);
	if (p_buf_index >= BUFFER_SIZE) {
		p_buf_index %= BUFFER_SIZE;
	}
	if (p_buf_index >= BUFFER_SIZE) {
		printf("error producing %d to buffer\n", item);
	} else {
		buffer[p_buf_index] = item;
		p_buf_index++;
		printf("producer %d produced %d to buffer\n", param, item);
   }
	pthread_mutex_unlock(&mutex);
   sem_post(&full);
}

/**
 * Removes an item from the buffer with the protection of 
 * semaphore and mutex
 *
 * @param item 	Random number to be inserted into the buffer
 * @param param	Locally generated thread ID 
 */

void remove_item(buffer_item item, void *param) {
   sem_wait(&full);
   pthread_mutex_lock(&mutex);
	if (c_buf_index >= BUFFER_SIZE) {
		c_buf_index %= BUFFER_SIZE;
	}
	if (c_buf_index < 0) {
		printf("error consuming %d from buffer\n", item);
	} else {
		item = buffer[c_buf_index];
		printf("consumer %d consumed %d from buffer\n", param, item);
	}
	buffer[c_buf_index] = 0;
	c_buf_index++;
   pthread_mutex_unlock(&mutex);
   sem_post(&empty);
}

/**
 * Creates a random number to insert into the buffer
 *
 * @param param	Locally generated thread ID 
 */

void *producer(void *param) {
	buffer_item item;
	int seed = rand();
	
	while (1) {
		sleep(rand() % 10 + 1);
		item = rand_r(&seed);
		insert_item(item, param);
	}
}

/**
 * Removes a number from the buffer
 *
 * @param param	Locally generated thread ID 
 */
 
void *consumer(void *param) {
	buffer_item item;
	
	while (1) {
		sleep(rand() % 10 + 1);
		remove_item(item, param);
	}
}