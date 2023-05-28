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
#include <semaphore.h>


// Global variables
int num_of_producers;
int co_editor_Q_size;
//create a global array of pointers to the producer's queues
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
    pthread_mutex_t m;
    sem_t full;
    sem_t empty;
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
    bounded_queue->queue_array = (struct news *) malloc((queue_size-1) * sizeof(struct news));
    bounded_queue->num_of_elements = queue_size;
    bounded_queue->head_index = 0;
    bounded_queue->tail_index = 0;
    // initialize the mutex
    pthread_mutex_init(&bounded_queue->m, NULL);
    // initialize the semaphores
    sem_init(&bounded_queue->full, 0, 0);
    sem_init(&bounded_queue->empty, 0, queue_size);
    return bounded_queue;
}


// This function adds an element to the queue
bool enqueueBoundedQueue(struct BoundedQueue *queue, struct news *n) {
    printf("enqueue head index=%d, tail index=%d, thread=%ld\n", queue->head_index, queue->tail_index, pthread_self());
    // check if the queue is uninitialized
    if (queue == NULL) {
        printf("Queue is uninitialized, thread=%ld\n", pthread_self());
        return false;
    }
    // Acquire the empty semaphore to check if the queue is full
    sem_wait(&queue->empty);
    // Lock the mutex to protect the critical section
    pthread_mutex_lock(&queue->m);
    // Add the news struct into the queue
    queue->queue_array[queue->tail_index] = *n;
    // Update the tail index and handle wrap-around
    queue->tail_index = (queue->tail_index + 1) % queue->num_of_elements;
    // Release the mutex and signal that the queue is not empty
    pthread_mutex_unlock(&queue->m);
    sem_post(&queue->full);
    return true;
}


// This function removes an element from the queue
bool dequeueBoundedQueue(struct BoundedQueue *queue, struct news *n){
    printf("dequeue head index=%d, tail index=%d, thread=%ld\n", queue->head_index, queue->tail_index, pthread_self());
    // check if the queue is uninitialized
    if (queue == NULL) {
        printf("Queue is uninitialized, thread=%ld\n", pthread_self());
        return false;
    }
    // Acquire the full semaphore to check if the queue is empty
    sem_wait(&queue->full);
    // Lock the mutex to protect the critical section
    pthread_mutex_lock(&queue->m);
    // Retrieve the news struct from the queue
    *n = queue->queue_array[queue->head_index];
    // Update the head index and handle wrap-around
    queue->head_index = (queue->head_index + 1) % queue->num_of_elements;
    // Release the mutex and signal that the queue is not full
    pthread_mutex_unlock(&queue->m);
    sem_post(&queue->empty);
    return true;
}


void *producer(void *producer_info_p){
    //sleep for 0.5 seconds
//    usleep(500000);
    struct producer_info *p = (struct producer_info *) producer_info_p;
    // create a queue for the producer in the global array
    struct BoundedQueue *queue = create_queue(p->queue_size);;
    printf("Done creating queue number %d, thread=%ld\n", p->producer_id ,pthread_self());
    // add the queue to the global array
    producers_queues[p->producer_id-1] = queue;
    char *types[3] = {"SPORTS", "NEWS", "WEATHER"};
    int i = 0;
    bool enqueue = false;
    while (i < p->num_of_products) {
        struct news *n = (struct news *) malloc(sizeof(struct news));
        n->producer_id = p->producer_id;
        n->product_number = i;
        n->type = types[rand()%3];
        enqueue = enqueueBoundedQueue(queue, n);
        printf("producer, i=%d, thread num=%ld\n", i, pthread_self());
        if (enqueue){
            i++;
        }
    }
    while(1) {
        // when the producer is done adding all the products, it adds a done news struct to the queue
        struct news *done = (struct news *) malloc(sizeof(struct news));
        done->producer_id = p->producer_id;
        done->product_number = -1;
        done->type = "DONE";
        enqueue = enqueueBoundedQueue(queue, done);
        if (!enqueue){
            printf("enqueue failed %ld\n", pthread_self());
        } else {
            printf("Producer %d has finished producing %d products\n", p->producer_id, p->num_of_products);
            // close the semaphores
            sem_close(&queue->full);
            sem_close(&queue->empty);
            // exit the thread
            break;
        }
    }
    // End of producer thread
}


void *dispatcher(void *d){
    // create an array to store all the news
    int counter = num_of_producers;
    while (counter != 0) {
        printf("dispatcher, counter %d, thread=%ld\n", counter, pthread_self());
        // RR the producers queues until they are all empty
        for (int i = 0; i < num_of_producers; i++){
            bool dequeue = false;
            while (!dequeue) {
                // dequeue the news struct from the queue
                struct news *n = (struct news *) malloc(sizeof(struct news));
                if (producers_queues[i] == NULL){
                    break;
                } else {
                    dequeue = dequeueBoundedQueue(producers_queues[i], n);
                    if (dequeue) {
                        if (strcmp(n->type, "DONE") == 0){
                            printf("DONE\n");
                            producers_queues[i] = NULL;
                            counter--;
                        } else {
                            printf("news type=%s\n", n->type);
                        }
                    }
                    // free n
                    free(n);
                }
            }
        }
    }
}



// This is the main program
// The program receives a configuration file as argument and store the file data into global variables.
// Then it creates a thread for each producer in the producer queues and runs the producer function.
int main(int argc, char **argv) {
    // Check if the user has provided a configuration file
    if (argc < 2) {
        printf("Please provide a configuration file.\n");
        return -1;
    }
    // call the read_file function with an empty struct array
    struct producer_info *pi;
    pi = (struct producer_info *) malloc(100);
    // send the file name and the empty array to read file function to fill the struct array
    read_file(argv[1], pi);
    // create an array of pointers to bounded queues
    producers_queues = (struct BoundedQueue **) malloc(num_of_producers * sizeof(struct BoundedQueue *));
    printf("The main thread id= %ld\n", pthread_self());
    //create a new producer thread for each producer to run the producer function
    for (int i = 0; i < num_of_producers; i++){
        // create a producer thread
        pthread_t producer_thread;
        pthread_create(&producer_thread, NULL, producer, &pi[i]);
    }
    // create a dispatcher thread
    pthread_t dispatcher_thread;
    pthread_create(&dispatcher_thread, NULL, dispatcher, NULL);
    // wait for the dispatcher thread to finish
    pthread_join(dispatcher_thread, NULL);
    //free pi array
    printf("the program has finished\n");
    free(pi);
}
