/* Author: John Lorenz Salva
 *
 * simulate.c simulates FIFO, LRU, and OPT
 * page replacement algorithms.
 * 
 * Date: Spring 2020
 */

#include <stdio.h>
#include <stdlib.h>
#define BUFFER_LENGTH 50

typedef struct Node {
   int page;
   struct Node* next;
} Node;

typedef struct List {
   struct Node* head;
	int size;
} List;

FILE *open_file(char *file);
void add_to_list(List *pages, int page);
void init(List *pages, FILE *input, int page_num, int num_of_requests);
void init_array(int frame[], int size);
void FIFO(int frame[], int size, List *requests);
void LRU(int frame[], int size, List *requests);
void OPT(int frame[], int size, List *requests);
int find_opt_replace_index(int page, Node *current);
int find_LRU_replace_index(int page, Node *current, List* requests);

/**
 * Main function
 */

int main(int argc, char *argv[]) {
	
	List *requests = (List*) malloc(sizeof(List));
	char page_num_data[BUFFER_LENGTH];
   char frame_data[BUFFER_LENGTH];
   char request_data[BUFFER_LENGTH];
	char data[BUFFER_LENGTH];
	FILE *input;
	
	if (argc <= 2) {
		printf("\nUsage: simulate <input_file> FIFO|LRU|OPT\n\n");
		free(requests);
		return EXIT_FAILURE;
   } else {
		input = open_file(argv[1]);
		
		fgets(data, sizeof data, input);
		sscanf(data, "%s%s%s", page_num_data, frame_data, request_data);
		if (atoi(frame_data) == 0) {
			printf("ERROR: Cannot have 0 frames as an input\n");
			return EXIT_FAILURE;
		}
		int num_of_pages = atoi(page_num_data);
		int frame[atoi(frame_data)];
		int frame_size = atoi(frame_data);
		int num_of_requests = atoi(request_data);
		
		init(requests, input, num_of_pages, num_of_requests);
		init_array(frame, frame_size);
		
		if (strcmp(argv[2], "FIFO") == 0) {
			FIFO(frame, frame_size, requests);
		} else if (strcmp(argv[2], "LRU") == 0) {
			LRU(frame, frame_size, requests);
		} else if (strcmp(argv[2], "OPT") == 0) {
			OPT(frame, frame_size, requests);
		} else {
			printf("\nUsage: simulate <input_file> FIFO|LRU|OPT\n\n");
			free(requests);
			return EXIT_FAILURE;
		}
	}
	
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
 * Initializes the page sequence list
 **/

void init(List *pages, FILE *input, int page_num, int num_of_requests) {
   char data[BUFFER_LENGTH];
	int requests = 0;
   while (fgets(data, sizeof data, input)) {
		requests++;
      char page_data[BUFFER_LENGTH];
      sscanf(data, "%s", page_data);
		int page = atoi(page_data);
		if (page > page_num || page < 0) {
			printf("ERROR: Page %d cannot be added because it falls outside of the # of pages\n", page);
			exit(EXIT_FAILURE);
		}
      add_to_list(pages, page);
   }
	if (requests != num_of_requests) {
		printf("ERROR: # of page requests do not match # of page requests specified in the file\n");
		exit(EXIT_FAILURE);
	}
}

/**
 * Initializes the array with -1 values
 * meaning empty
 **/

void init_array(int frame[], int size) {
	int i;
	for (i = 0; i < size; i++)
		frame[i] = -1;
}

/**
 * Queues the given page into the list
 **/

void add_to_list(List *pages, int page) {
   Node *new = (Node *) malloc(sizeof(Node));
	new->page = page;
	if (pages->head == NULL) {
		pages->head = new;
	} else {
		Node *prev = pages->head;
		Node *temp = pages->head->next;
		while(temp != NULL) {
			prev = temp;
			temp = temp->next;
		}
		prev->next = new;
	}
}

/**
 * Simulates the FIFO page replacement algorithm
 **/

void FIFO(int frame[], int size, List *requests) {
	Node *temp = requests->head;
	int replace_index = 0;
	int i;
	int page_faults = 0;
	int frame_detected = 0;
	int page_fault_flag = 1;

	while(temp != NULL) {
		for (i = 0; i < size; i++) {
			if (temp->page == frame[i]) {
				page_fault_flag = 0;
				frame_detected = i;
				break;
			}
		}
		if(page_fault_flag) {
			if (frame[replace_index] == -1) {
				frame[replace_index] = temp->page;
				printf("Page %d loaded into Frame %d\n", temp->page, replace_index);
			} else {
				printf("Page %d unloaded into Frame %d, ", frame[replace_index], replace_index);
				frame[replace_index] = temp->page;
				printf("Page %d loaded into Frame %d\n", temp->page, replace_index);
			}
			if (replace_index + 1 == size)
				replace_index = 0;
			else
				replace_index++;
			page_faults++;
		} else {
			printf("Page %d already in Frame %d\n", temp->page, frame_detected);
			page_fault_flag = 1;
		}
		temp = temp->next;
	}
	printf("%d page faults\n", page_faults);
}

/**
 * Simulates the OPT page replacement algorithm
 **/

void OPT(int frame[], int size, List *requests) {
	
	int frame_detected = 0;
	int i;
	int index_replace_time = 0;
	int longest_replace_time = 0;
	int replace_index = 0;
	int page_faults = 0;
	int page_fault_flag = 1;
	Node *temp = requests->head;

	while(temp != NULL) {
		for (i = 0; i < size; i++) {
			if (temp->page == frame[i]) {
				page_fault_flag = 0;
				frame_detected = i;
				break;
			}
		}
		if(page_fault_flag) {
			if (frame[replace_index] == -1) {
				frame[replace_index] = temp->page;
				printf("Page %d loaded into Frame %d\n", temp->page, replace_index);
				replace_index++;
			} else {
				for (i = 0; i < size; i++) {
					index_replace_time = find_opt_replace_index(frame[i], temp);
					if (longest_replace_time < index_replace_time) {
						replace_index = i;
						longest_replace_time = index_replace_time;
					}
				}
				printf("Page %d unloaded from Frame %d, ", frame[replace_index], replace_index);
				frame[replace_index] = temp->page;
				printf("Page %d loaded into Frame %d\n", temp->page, replace_index);
				longest_replace_time = 0;
			}
			page_faults++;
		} else {
			printf("Page %d already in Frame %d\n", temp->page, frame_detected);
			page_fault_flag = 1;
		}
		temp = temp->next;
	}
	printf("%d page faults\n", page_faults);
}

/**
 * Simulates the LRU page replacement algorithm
 **/

void LRU(int frame[], int size, List *requests) {
	
	int frame_detected = 0;
	int index_replace_time = 0;
	int i;
	int longest_replace_time = 0;
	int replace_index = 0;
	int page_faults = 0;
	int page_fault_flag = 1;
	Node *temp = requests->head;

	while(temp != NULL) {
		for (i = 0; i < size; i++) {
			if (temp->page == frame[i]) {
				page_fault_flag = 0;
				frame_detected = i;
				break;
			}
		}
		if(page_fault_flag) {
			if (frame[replace_index] == -1) {
				frame[replace_index] = temp->page;
				printf("Page %d loaded into Frame %d\n", temp->page, replace_index);
				replace_index++;
			} else {
				for (i = 0; i < size; i++) {
					index_replace_time = find_LRU_replace_index(frame[i], temp, requests);
					if (longest_replace_time < index_replace_time) {
						replace_index = i;
						longest_replace_time = index_replace_time;
					}
				}
				printf("Page %d unloaded from Frame %d, ", frame[replace_index], replace_index);
				frame[replace_index] = temp->page;
				printf("Page %d loaded into Frame %d\n", temp->page, replace_index);
				longest_replace_time = 0;
			}
			page_faults++;
		} else {
			printf("Page %d already in Frame %d\n", temp->page, frame_detected);
			page_fault_flag = 1;
		}
		temp = temp->next;
	}
	printf("%d page faults\n", page_faults);
}

/**
 * Searches for the next index to be replaced
 * based on when the page will be accessed in the future
 **/

int find_opt_replace_index(int page, Node *current) {
	Node *temp = current->next;
	int time = 10000000;
	int jumps = 1;
	while(temp != NULL) {
		if (temp->page == page) {
			time = jumps;
			break;
		} else {
			jumps++;
		}
		temp = temp->next;
	}
	
	return time;
}

/**
 * Searches for the next index to be replaced
 * based on past information about the pages accessed
 **/

int find_LRU_replace_index(int page, Node *current, List* requests) {
	Node *temp = requests->head;
	int time = 0;
	int jumps = 0;
	while(temp != current) {
		if (temp->page == page)
			time = jumps;
		jumps++;
		temp = temp->next;
	}
	
	return jumps - time;
}