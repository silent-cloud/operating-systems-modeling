/**
 * Author: John Lorenz Salva
 * CSC-139 Section 02
 *
 * Replicates the CPU scheduling algorithms by reading an
 * input file that follows the following format:
 *       pid arrival_time burst_time
 **/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#define BUFFER_LENGTH 50

/**
 * Process Control Block structure
 **/

typedef struct {
   int pid;
   int a_time;
   int b_time;
	int response_flag;
	int time_halted;
} PCB_t;

/**
 * Node structure
 **/

typedef struct Node {
   PCB_t PCB;
   struct Node* next;
} Node;

/**
 * Queue structure
 **/

typedef struct Queue {
   struct Node* head;
   struct Node* tail;
	int size;
} Queue;

/**
 * Function prototypes
 **/

FILE *open_file(char *file);
int next_arrival_time(Queue *job);
int next_burst_time(Queue *ready);
PCB_t dispatch(Queue* job);
PCB_t dequeue_process(Queue *ready);
void init(Queue *q, char *file);
void add_to_job_list(Queue *q, PCB_t PCB);
void FCFS(Queue *job, Queue *ready);
void SRTF(Queue *job, Queue *ready);
void RR(Queue *job, Queue *ready, int quantum);
void insert_before(Node *prev, Node *new, Node *before);
void insert_after(Node *new, Node *current);
void enqueue_process(Queue *q, PCB_t PCB);
void insert_in_order(Queue *q, PCB_t PCB);

/**
 * Main function
 * 
 * Decides what scheduling algorithm to use based on user
 * input. Incorrect usages of the command output error
 * usage messages and exits the program.
 **/

int main(int argc, char *argv[]) {
	char start;
	int quantum;
	Queue *job = (Queue*) malloc(sizeof(Queue));
	Queue *ready = (Queue*) malloc(sizeof(Queue));

   if (argc > 2) {
      if (strcmp(argv[2], "FCFS") == 0 && argv[3] == NULL) {
			init(job, argv[1]);
			printf("Total of %d tasks were read from \"%s\". Press \"Enter\" to start...\n", job->size, argv[1]);
			printf("===================================================================\n");
			while (start != '\r' && start != '\n')
				start = getchar();
         FCFS(job, ready);
			free(job);
			free(ready);
      } else if (strcmp(argv[2], "SRTF") == 0 && argv[3] == NULL) {
			init(job, argv[1]);
			printf("Total of %d tasks were read from \"%s\". Press \"Enter\" to start...\n", job->size, argv[1]);
			printf("===================================================================\n");
			while (start != '\r' && start != '\n')
				start = getchar();
         SRTF(job, ready);
      } else if (strcmp(argv[2], "RR") == 0 && argv[3] != NULL) {
			//Checks if the time quantum is an explicit number
			char *quantum_data = argv[3];
			int i;
			for (i = 0; quantum_data[i] != '\0'; i++) {
				if (!isdigit(quantum_data[i])) {
					printf("\nUsage: schedule <input_file> FCFS|STRF|RR [quantum]\n\n");
					free(job);
					free(ready);
					return EXIT_FAILURE;
				}
			}
			init(job, argv[1]);
			printf("Total of %d tasks were read from \"%s\". Press \"Enter\" to start...\n", job->size, argv[1]);
			printf("===================================================================\n");
			while (start != '\r' && start != '\n')
				start = getchar();
			quantum = atoi(argv[3]);
			RR(job, ready, quantum);
		} else {
			printf("\nUsage: schedule <input_file> FCFS|SRTF|RR [quantum]\n\n");
			free(job);
			free(ready);
			return EXIT_FAILURE;
		}
   } else {
		printf("\nUsage: schedule <input_file> FCFS|SRTF|RR [quantum]\n\n");
		free(job);
		free(ready);
		return EXIT_FAILURE;
	}
	
	free(job);
	free(ready);
   return EXIT_SUCCESS;
}

/**
 * Opens a file passed to the function
 **/

FILE *open_file(char *file) {
   FILE *ptr_file = fopen(file, "r");
   if (ptr_file == NULL) {
      printf("Error on fopen %s \n", file);
      exit(EXIT_FAILURE);
   }
   return ptr_file;
}

/**
 * Initializes the job list to dispatch processes
 * in order.
 **/

void init(Queue *job, char *file) {
   char data[BUFFER_LENGTH];
	PCB_t new_PCB;
   FILE *input = open_file(file);
   while (fgets(data, sizeof data, input)) {
      char pid_data[BUFFER_LENGTH];
      char a_time_data[BUFFER_LENGTH];
      char b_time_data[BUFFER_LENGTH];
      sscanf(data, "%s%s%s", pid_data, a_time_data, b_time_data);
      new_PCB.pid = atoi(pid_data);
      new_PCB.a_time = atoi(a_time_data);
      new_PCB.b_time = atoi(b_time_data);
		new_PCB.response_flag = 0;
		new_PCB.time_halted = 0;
      add_to_job_list(job, new_PCB);
		job->size++;
   }
}

/**
 * Inserts a new process added to the sorted job list
 **/
 
void add_to_job_list(Queue *q, PCB_t PCB) {
   Node *new = (Node *) malloc(sizeof(Node));
	Node *temp = q->head;
	Node *prev = q->head;
   new->PCB = PCB;
   //Queue is empty
   if (q->head == NULL) {
      q->head = new;
   } else {
		//New process is arrives earlier than the first entry in the list
		if (new->PCB.a_time < q->head->PCB.a_time) {
			insert_before(NULL, new, q->head);
			q->head = new;
		//New process is arrives at the same time as first entry in the list
		} else if (new->PCB.a_time == q->head->PCB.a_time) {
				insert_after(new, q->head);
		//New process needs to be inserted after the first entry in the list
		} else {
			temp = temp->next;
			//Traverse until we hit the end
			while(temp != NULL) {
				//New process arrives earlier than the currently-pointed entry in the list
				if (new->PCB.a_time < temp->PCB.a_time) {
					insert_before(prev, new, temp);
					break;
				//New process arrives at the same time as the currently-pointed entry in the list
				} else if (new->PCB.a_time == temp->PCB.a_time) {
					//New process' id less than first entry's pid meaning higher priority
					/*if(new->PCB.pid < q->head->PCB.pid) {
						insert_before(prev, new, temp);
						break;*/
					//New process' id greater than first entry's pid meaning lower priority
						insert_after(new, temp);
						break;
				//New process arrives later than the currently-pointed entry in the list
				} else {
					prev = temp;
					temp = temp->next;
				}
			}
			//New process arrives latest, place at the end of the list
			if (temp == NULL) {
				prev->next = new;
			}
		}
	}
}

/**
 * Simulates the First Come, First Serve CPU scheduling
 * algorithm using the initialized job queue and empty
 * ready queue
 **/

void FCFS(Queue *job, Queue *ready) {
	
	double avg_cpu_time = 0.0;				// Measures CPU usage
	double avg_wait_time = 0.0;			// Average wait time
	double avg_response_time = 0.0;		// Average response time
	double avg_turnaround_time = 0.0;	// Average turnaround time
	int total_processes = job->size;		// Amount of processes to work on
	int system_time = 0;						// System time in milliseconds
	int idle_cpu_time = 0;					// CPU time spent in idle mode
	
	// Continue running while both job list and ready queue are not empty
	while(job->head != NULL || ready->head != NULL) {
		// Enqueue all processes that arrives at this current system time to the ready queue
		while (job->head != NULL && system_time == next_arrival_time(job)) {
			enqueue_process(ready, dispatch(job));
			ready->size++;
			job->size--;
		}
		// Processes are ready to be executed
		if (ready->head != NULL) {
			PCB_t process = dequeue_process(ready);
			// Process first time running
			if (process.response_flag == 0) {
				process.response_flag = 1;
				avg_response_time += (system_time - process.a_time);
			}
			avg_wait_time += (system_time - process.a_time);
			ready->size--;
			// Process is running
			while (process.b_time > 0) {
				// Enqueue processes that arrive to the ready queue while the process is running
				while (job->head != NULL && system_time == next_arrival_time(job)) {
					enqueue_process(ready, dispatch(job));
					ready->size++;
					job->size--;
				}
				printf("<system time\t%d> process\t%d is running\n", system_time, process.pid);
				sleep(1);
				process.b_time--;
				system_time++;
			}
			printf("<system time\t%d> process\t%d is finished.......\n", system_time, process.pid);
			avg_turnaround_time += (system_time - process.a_time);
		// Ready is empty but the process has not arrived yet
		} else {
			printf("<system time\t%d> CPU idle........\n", system_time);
			idle_cpu_time++;
			system_time++;
		}
	}
	
	avg_cpu_time = ((double) (system_time - idle_cpu_time) / (double) system_time) * 100.0;
	avg_wait_time /= total_processes;
	avg_response_time /= total_processes;
	avg_turnaround_time /= total_processes;
	
	printf("<system time\t%d> All processes finish.........\n", system_time);
	printf("\n=====================================================\n");
	printf("Average CPU usage\t: %.2f\%\n", avg_cpu_time);
	printf("Average waiting time\t: %.2f ms\n", avg_wait_time);
	printf("Average response time\t: %.2f ms\n", avg_response_time);
	printf("Average turnaround time\t: %.2f ms\n", avg_turnaround_time);
	printf("=====================================================\n\n");
}

/**
 * Simulates the Shortest Remaining Time First CPU scheduling
 * algorithm using the initialized job queue and empty
 * ready queue
 **/

void SRTF(Queue *job, Queue *ready) {

	double avg_cpu_time = 0.0;				// Measures CPU usage
	double avg_wait_time = 0.0;			// Average wait time
	double avg_response_time = 0.0;		// Average response time
	double avg_turnaround_time = 0.0;	// Average turnaround time
	int total_processes = job->size;		// Amount of processes to work on
	int system_time = 0;						// System time in milliseconds
	int idle_cpu_time = 0;					// CPU time spent in idle mode

	// Continue running while both job list and ready queue are not empty
	while(job->head != NULL || ready->head != NULL) {
		// Insert all processes that arrives at this current system time to the ready queue in order
		while (job->head != NULL && system_time == next_arrival_time(job)) {
			insert_in_order(ready, dispatch(job));
			ready->size++;
			job->size--;
		}
		// Processes are ready to be executed
		if (ready->head != NULL) {
			PCB_t process = dequeue_process(ready);
			// Process first time running
			if (process.response_flag == 0) {
				process.response_flag = 1;
				avg_response_time += (system_time - process.a_time);
				avg_wait_time += (system_time - process.a_time);
			} else {
				avg_wait_time += (system_time - process.time_halted);
			}
			ready->size--;
			// Process is running
			while (process.b_time > 0) {
				// Insert processes that arrive to the ready queue in order while the process is running
				while (job->head != NULL && system_time == next_arrival_time(job)) {
					insert_in_order(ready, dispatch(job));
					ready->size++;
					job->size--;
				}
				// Preempt the currently running process if the next process to run is shorter to run.
				if(next_burst_time(ready) >=  0 && next_burst_time(ready) < process.b_time) {
					process.time_halted = system_time;
					insert_in_order(ready, process);
					process = dequeue_process(ready);
				}
				printf("<system time\t%d> process\t%d is running\n", system_time, process.pid);
				sleep(1);
				process.b_time--;
				system_time++;
			}
			printf("<system time\t%d> process\t%d is finished.......\n", system_time, process.pid);
			avg_turnaround_time += (system_time - process.a_time);
		// Ready is empty but the process has not arrived yet
		} else {
			printf("<system time\t%d> CPU idle........\n", system_time);
			idle_cpu_time++;
			system_time++;
		}
	}
	
	avg_cpu_time = ((double) (system_time - idle_cpu_time) / (double) system_time) * 100.0;
	avg_wait_time /= total_processes;
	avg_response_time /= total_processes;
	avg_turnaround_time /= total_processes;
	
	printf("<system time\t%d> All processes finish.........\n", system_time);
	printf("\n=====================================================\n");
	printf("Average CPU usage\t: %.2f\%\n", avg_cpu_time);
	printf("Average waiting time\t: %.2f ms\n", avg_wait_time);
	printf("Average response time\t: %.2f ms\n", avg_response_time);
	printf("Average turnaround time\t: %.2f ms\n", avg_turnaround_time);
	printf("=====================================================\n\n");
}

/**
 * Simulates the Round Robin CPU scheduling
 * algorithm using the initialized job queue and empty
 * ready queue
 **/

void RR(Queue *job, Queue *ready, int quantum) {

	double avg_cpu_time = 0.0;				// Measures CPU usage
	double avg_wait_time = 0.0;			// Average wait time
	double avg_response_time = 0.0;		// Average response time
	double avg_turnaround_time = 0.0;	// Average turnaround time
	int total_processes = job->size;		// Amount of processes to work on
	int system_time = 0;						// System time in milliseconds
	int idle_cpu_time = 0;					// CPU time spent in idle mode
	int run_time = 0;								// Time that the process has spent running

	// Continue running while both job list and ready queue are not empty
	while(job->head != NULL || ready->head != NULL) {
		// Enqueue all processes that arrives at this current system time to the ready queue
		while (job->head != NULL && system_time == next_arrival_time(job)) {
			enqueue_process(ready, dispatch(job));
			ready->size++;
			job->size--;
		}
		// Processes are ready to be executed
		if (ready->head != NULL) {
			PCB_t process = dequeue_process(ready);
			// Process first time running
			if (process.response_flag == 0) {
				process.response_flag = 1;
				avg_response_time += (system_time - process.a_time);
				avg_wait_time += (system_time - process.a_time);
			} else {
				avg_wait_time += (system_time - process.time_halted);
			}
			ready->size--;
			// Process is running
			while (process.b_time > 0) {
				// Enqueue processes that arrive to the ready queue while the process is running
				while (job->head != NULL && system_time == next_arrival_time(job)) {
					enqueue_process(ready, dispatch(job));
					ready->size++;
					job->size--;
				}
				// Preempt the currently running process with the next process 
				// if the current process has run out running time.
				if(run_time == quantum) {
					process.time_halted = system_time;
					run_time = 0;
					enqueue_process(ready, process);
					process = dequeue_process(ready);
					// Process first time running
					if (process.response_flag == 0) {
						process.response_flag = 1;
						avg_response_time += (system_time - process.a_time);
						avg_wait_time += (system_time - process.a_time);
					} else {
						avg_wait_time += (system_time - process.time_halted);
					}
				}
				printf("<system time\t%d> process\t%d is running\n", system_time, process.pid);
				sleep(1);
				run_time++;
				process.b_time--;
				system_time++;
			}
			printf("<system time\t%d> process\t%d is finished.......\n", system_time, process.pid);
			run_time = 0;
			avg_turnaround_time += (system_time - process.a_time);
		// Ready is empty but the process has not arrived yet
		} else {
			printf("<system time\t%d> CPU idle........\n", system_time);
			idle_cpu_time++;
			system_time++;
		}
	}
	
	avg_cpu_time = ((double) (system_time - idle_cpu_time) / (double) system_time) * 100.0;
	avg_wait_time /= total_processes;
	avg_response_time /= total_processes;
	avg_turnaround_time /= total_processes;
	
	printf("<system time\t%d> All processes finish.........\n", system_time);
	printf("\n=====================================================\n");
	printf("Average CPU usage\t: %.2f\%\n", avg_cpu_time);
	printf("Average waiting time\t: %.2f ms\n", avg_wait_time);
	printf("Average response time\t: %.2f ms\n", avg_response_time);
	printf("Average turnaround time\t: %.2f ms\n", avg_turnaround_time);
	printf("=====================================================\n\n");
}

/**
 * Inserts the new node before the current node
 **/

void insert_before(Node *prev, Node *new, Node *current) {
	// New node is not inserted before the head
	if (prev != NULL) {
		new->next = current;
		prev->next = new;
	} else {
		new->next = current;
	}
}

/**
 * Inserts the new node after the current node
 **/

void insert_after(Node *new, Node *current) {
	// New node is not inserted after the last entry
	if(current->next != NULL) {
		new->next = current->next;
		current->next = new;
		// New node shares the same process arrival time as the next node
		if (new->PCB.a_time == new->next->PCB.a_time) {
				current->next = new->next;
				insert_after(new, new->next);
		}			
	} else {
		current->next = new;
		new->next = NULL;
	}
}

/**
 * Dispatches the next process by removing the process
 * from the job list.
 **/

PCB_t dispatch(Queue* job) {
	Node *temp = job->head;
	PCB_t PCB = job->head->PCB;
	// Last process in the job list
	if(job->head->next != NULL) {
		job->head = job->head->next;
		temp->next = NULL;
		free(temp);
	} else {
		job->head = NULL;
		free(temp);
	}
	return PCB;
}

/**
 * Gives the next process' arrival time in the job list
 **/

int next_arrival_time(Queue *job) {
	if(job->head != NULL)
		return job->head->PCB.a_time;
	else
		return -1;
}

/**
 * Gives the next process' burst time in the ready queue
 **/

int next_burst_time(Queue *ready) {
	if(ready->head != NULL)
		return ready->head->PCB.b_time;
	else
		return -1;
}

/**
 * Enqueue the next process to run in the ready queue
 **/

void enqueue_process(Queue *ready, PCB_t PCB) {
   Node *temp = (Node *) malloc(sizeof(Node));
   temp->PCB = PCB;
   if (ready->head == NULL)
      ready->head = temp;
   else
      ready->tail->next = temp;
   ready->tail = temp;
   ready->tail->next = ready->head;
}

/**
 * Insert the next process to run in the ready queue in order
 **/

void insert_in_order(Queue *q, PCB_t PCB) {
   Node *new = (Node *) malloc(sizeof(Node));
	Node *temp = q->head;
	Node *prev = q->head;
   new->PCB = PCB;
   // Ready queue is empty
   if (q->head == NULL) {
      q->head = new;
		q->tail = new;
		q->tail->next = new;
   } else {
		//New process' burst time is less than the first entry in the queue
		if (new->PCB.b_time < q->head->PCB.b_time) {
			insert_before(NULL, new, q->head);
			q->head = new;
			q->tail->next = q->head;
		//New process' burst time is the same as the first entry in the queue
		} else if (new->PCB.b_time == q->head->PCB.b_time) {
			// New process is the last entry of the queue
			if (q->head == q->tail)
				q->tail = new;
			insert_after(new, q->head);
		//New process needs to be inserted after the first entry in the queue
		} else {
			temp = temp->next;
			//Traverse until we loop back to the beginning
			while(temp != q->head) {
				//New process' burst time is less than the currently-pointed entry in the queue
				if (new->PCB.b_time < temp->PCB.b_time) {
					insert_before(prev, new, temp);
					break;
				//New process' burst time is the same as the currently-pointed entry in the queue
				} else if (new->PCB.b_time == temp->PCB.b_time) {
						insert_after(new, temp);
						break;
				//New process arrives later than the currently-pointed entry in the list
				} else {
					prev = temp;
					temp = temp->next;
				}
			}
			//New process arrives latest, place at the end of the list
			if (temp == q->head) {
				new->next = q->head;
				prev->next = new;
				q->tail = new;
			}
		}
	}
}

/**
 * Dequeues the next process to run in the ready queue
 **/

PCB_t dequeue_process(Queue *ready) {
	Node *temp = ready->head;
	PCB_t PCB = ready->head->PCB;
	//	Last process to run in the ready queue
   if (ready->head == ready->tail) {
		ready->head->next = NULL;
		ready->tail->next = NULL;
		free(temp);
		ready->head = NULL;
		ready->tail = NULL;
		return PCB;
   } else {
      ready->head = ready->head->next;
		ready->tail->next = ready->head;
		free(temp);
		return PCB;
	}
}
