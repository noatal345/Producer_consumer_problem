#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>
#include <time.h>
#include <pthread.h>


// Global variables
int num_of_producers;
int co_editor_Q_size;
//create a global array of pointers to the queues
struct BoundedQueue **producers_queues;

// Structs
struct producer_info{
    int producer_id;
    int num_of_products;
    int queue_size;
};

struct news{
    int producer_id;
    int product_number;
    char *type;
};

struct BoundedQueue{
    struct news *queue_array;
    int num_of_elements;
    int head_index;
    int tail_index;
};


//Function declarations
bool read_file(char *file_name, struct producer_info *pi);

// This function reads the configuration file and stores the data by the following roles:
// while the file isn't over store every 3 lines in a producer_info struct, where:
// 1st line is the producer id
// 2nd line is the number of products
// 3rd line is the queue size
// the last line of the file is the size of the co-editor queue.
bool read_file(char *file_name, struct producer_info *pi) {
    // open the configuration file for reading
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        perror("Error in: fopen");
        return false;
    }

    int producer_num = 0;
    // read the file line by line
    while (1) {
        // read the first element into temp a variable to check if the file is over
        int temp;
        //read the producer id into temp
        fscanf(file, "%d", &temp);
        // if the file is over break the loop
        if (feof(file)) {
            co_editor_Q_size = temp;
            printf("%s\n", "done");
            break;
        } else {
            // if the file isn't over store the producer id in the producer_info struct
            pi[producer_num].producer_id = temp;
        }
        // read the number of products
        fscanf(file, "%d", &(pi[producer_num].num_of_products));
        // read the queue size
        fscanf(file, "%d", &(pi[producer_num].queue_size));
        //continue to the next producer
        producer_num++;
    }
    fclose(file);
    num_of_producers = producer_num;
    return true;
}

// This function creates a bounded queue with the given size
struct BoundedQueue *create_queue(int queue_size) {
    struct BoundedQueue *bounded_queue = (struct BoundedQueue *) malloc(sizeof(struct BoundedQueue));
    bounded_queue->queue_array = (struct news *) malloc(queue_size * sizeof(struct news));
    bounded_queue->num_of_elements = queue_size;
    bounded_queue->head_index = 0;
    bounded_queue->tail_index = 0;
    return bounded_queue;
}


// This function adds an element to the queue
void enqueueBoundedQueue(struct BoundedQueue *queue, struct news *n){
    if (queue->tail_index == queue->num_of_elements){
        queue->tail_index = 0;
    }
    if ((queue->tail_index == queue->head_index) && queue->tail_index != 0){
        printf("Queue is full\n");
        return;
    }
    // add the news struct into the queue
    queue->queue_array[queue->tail_index] = *n;
    queue->tail_index++;
}


// This function removes an element from the queue
void dequeueBoundedQueue(struct BoundedQueue *queue, struct news *n){
    if (queue->head_index == queue->num_of_elements){
        queue->head_index = 0;
    }
    if ((queue->head_index == queue->tail_index) && queue->head_index != 0){
        printf("Queue is empty\n");
        return;
    }
    *n = queue->queue_array[queue->head_index];
    queue->head_index++;
}


void *producer(struct producer_info *p){
    // create a queue for the producer in the global array
    struct BoundedQueue *queue = create_queue(p->queue_size);;
    // add the queue to the global array
    producers_queues[p->producer_id-1] = queue;
    char *types[3] = {"SPORTS", "NEWS", "WEATHER"};
    for(int i = 0; i < p->num_of_products; i++){
        struct news *n = (struct news *) malloc(sizeof(struct news));
        n->producer_id = p->producer_id;
        n->product_number = i;
        n->type = types[rand()%3];
        enqueueBoundedQueue(queue, n);
    }
}
// This is the main program
// The program receives a configuration file as argument and store the file data into global variables.
int main(int argc, char **argv) {
    // Check if the user has provided a configuration file
    if (argc < 2) {
        printf("Please provide a configuration file.\n");
        return -1;
    }
    // call the read_file function with an empty struct array
    struct producer_info *pi;
    pi = (struct producer_info *) malloc(100);
    // send the file name and the empty struct array
    read_file(argv[1], pi);
    // create an array of pointers to bounded queues
    producers_queues = (struct BoundedQueue **) malloc(num_of_producers * sizeof(struct BoundedQueue *));
    // create a new thread for each producer to run the producer function
    for (int i = 0; i < num_of_producers; i++){
        pthread_t thread;
        pthread_attr_t *attr = NULL;
        int pthread_create(thread, attr, producer, &pi[i]);
    }
    //free pi array
    free(pi);
    return 0;
}
