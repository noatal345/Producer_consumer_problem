//Noa Tal 209327279
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

//TODO delete prints

// Global variables
int num_of_producers;
//todo delete it's unbounded
int num_of_news;
int co_editors_Q_size;

//create a global arrays
struct BoundedQueue **producers_queues;
struct UnboundedQueue *sports_queue;
struct UnboundedQueue *weather_queue;
struct UnboundedQueue *news_queue;
struct BoundedQueue *co_editors_queue;

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

struct UnboundedQueue{
    struct news *queue_array;
    int num_of_elements;
    int size;
    int head_index;
    int tail_index;
    pthread_mutex_t m;
    sem_t full;
};


//Function declarations
bool read_file(char *file_name, struct producer_info *pi);
struct BoundedQueue *create_Bqueue(int queue_size);
bool enqueueBoundedQueue(struct BoundedQueue *queue, struct news *n);
bool dequeueBoundedQueue(struct BoundedQueue *queue, struct news *n);
struct UnboundedQueue *create_Uqueue(int s);
struct UnboundedQueue *increase_queue_array(struct UnboundedQueue *queue);
bool enqueueUnboundedQueue(struct UnboundedQueue *queue, struct news *n);
bool dequeueUnboundedQueue(struct UnboundedQueue *queue, struct news *n);
void *producer(void *producer_info_p);
void *dispatcher(void *d);
void *co_editor(void *news_type_queue);
void *screen_manager(void *screen_manager_queue);



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
    num_of_news = 0;
    int producer_num = 0;
    // read the file line by line
    while (1) {
        // read the first element into temp a variable to check if the file is over
        int temp;
        //read the producer id into temp
        fscanf(file, "%d", &temp);
        // if the file is over break the loop
        if (feof(file)) {
            co_editors_Q_size = temp;
            break;
        } else {
            // if the file isn't over store the producer id in the producer_info struct
            pi[producer_num].producer_id = temp;
        }
        // read the number of products
        fscanf(file, "%d", &(pi[producer_num].num_of_products));
        // read the queue size
        fscanf(file, "%d", &(pi[producer_num].queue_size));
        // update the number of news
        num_of_news += pi[producer_num].num_of_products;
        //continue to the next producer
        producer_num++;
    }
    fclose(file);
    num_of_producers = producer_num;
    return true;
}


// This function creates a bounded queue with a given size
struct UnboundedQueue *create_Uqueue(int s) {
    struct UnboundedQueue *unbounded_queue = (struct UnboundedQueue *) malloc(sizeof(struct UnboundedQueue));
    unbounded_queue->queue_array = (struct news *) malloc(sizeof(struct news) * s);
    unbounded_queue->num_of_elements = s;
    unbounded_queue->size = 0;
    unbounded_queue->head_index = 0;
    unbounded_queue->tail_index = 0;
    // initialize the mutex
    pthread_mutex_init(&unbounded_queue->m, NULL);
    // initialize the semaphores
    sem_init(&unbounded_queue->full, 0, 0);
    return unbounded_queue;
}


//This function Increase the Unbounded queue array if it's full
struct UnboundedQueue *increase_queue_array(struct UnboundedQueue *queue) {
    int new_size = queue->num_of_elements * 2;
    struct news *new_queue_array = (struct news *) malloc(sizeof(struct news) * new_size);
    for (int i = 0; i < queue->num_of_elements; i++) {
        new_queue_array[i] = queue->queue_array[i];
    }
    // lock this critical section
    pthread_mutex_lock(&queue->m);
    //free the old queue array
    free(queue->queue_array);
    queue->num_of_elements = new_size;
    queue->queue_array = new_queue_array;
    queue->tail_index = queue->size;
    // unlock the critical section
    pthread_mutex_unlock(&queue->m);
    return queue;
}


// This function adds an element to the queue
bool enqueueUnboundedQueue(struct UnboundedQueue *queue, struct news *n) {
    // check if the queue is uninitialized
    if (queue == NULL) {
        return false;
    }
    // check if the queue is full and increase the size of the queue if it is
    if (queue->size == queue->num_of_elements) {
        queue = increase_queue_array(queue);
    }
    // Lock the mutex to protect the critical section
    // Add the news struct into the queue
    // Update the tail index and handle wrap-around
    // Release the mutex and signal that the queue is not empty
    pthread_mutex_lock(&queue->m);
    queue->queue_array[queue->tail_index] = *n;
    queue->tail_index = (queue->tail_index + 1) % queue->num_of_elements;
    queue->size += 1;
    pthread_mutex_unlock(&queue->m);
    // increase the semaphore
    sem_post(&queue->full);
    return true;
}


// This function removes an element from the queue
bool dequeueUnboundedQueue(struct UnboundedQueue *queue, struct news *n) {
    // check if the queue is uninitialized
    if (queue == NULL) {
        return false;
    }
    // Lock the mutex to protect the critical section
    // Retrieve the news struct from the queue
    // Update the head index and handle wrap-around
    // Release the mutex and signal that the queue is not full
    //get semaphore value
    int sem_value;
    sem_getvalue(&queue->full, &sem_value);
    sem_wait(&queue->full);
    pthread_mutex_lock(&queue->m);
    *n = queue->queue_array[queue->head_index];
    queue->head_index = (queue->head_index + 1) % queue->num_of_elements;
    queue->size -= 1;
    pthread_mutex_unlock(&queue->m);
    return true;
}


// This function creates a bounded queue with a given size
struct BoundedQueue *create_Bqueue(int queue_size) {
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
    // check if the queue is uninitialized
    if (queue == NULL) {
        return false;
    }
    // Lock the mutex to protect the critical section
    // Add the news struct into the queue
    // Update the tail index and handle wrap-around
    // Release the mutex and signal that the queue is not empty
    sem_wait(&queue->empty);
    pthread_mutex_lock(&queue->m);
    queue->queue_array[queue->tail_index] = *n;
    queue->tail_index = (queue->tail_index + 1) % queue->num_of_elements;
    pthread_mutex_unlock(&queue->m);
    sem_post(&queue->full);
    return true;
}


// This function removes an element from the queue
bool dequeueBoundedQueue(struct BoundedQueue *queue, struct news *n) {
    // check if the queue is uninitialized
    if (queue == NULL) {
        return false;
    }
    // Lock the mutex to protect the critical section
    // Retrieve the news struct from the queue
    // Update the head index and handle wrap-around
    // Release the mutex and signal that the queue is not full
    sem_wait(&queue->full);
    pthread_mutex_lock(&queue->m);
    *n = queue->queue_array[queue->head_index];
    queue->head_index = (queue->head_index + 1) % queue->num_of_elements;
    pthread_mutex_unlock(&queue->m);
    sem_post(&queue->empty);
    return true;
}


// This function is the producer thread function
void *producer(void *producer_info_p) {
    struct producer_info *p = (struct producer_info *) producer_info_p;
    // create a queue for the producer in the global array
    struct BoundedQueue *queue = create_Bqueue(p->queue_size);;
    // add the queue to the global array
    producers_queues[p->producer_id-1] = queue;
    int type_counter[3] = {0, 0, 0};
    char *types[3] = {"SPORTS", "NEWS", "WEATHER"};
    bool enqueue = false;
    for(int i = 0; i < p->num_of_products; i++) {
        struct news *n = (struct news *) malloc(sizeof(struct news));
        n->producer_id = p->producer_id-1;
        int index = rand() % 3;
        n->type = types[index];
        n->product_number = type_counter[index];
        type_counter[index] += 1;
        while(!enqueueBoundedQueue(queue, n)){}
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
            // exit the thread
            break;
        }
    }
    // End of producer thread
}


// This function is the dispatcher(consumer) thread function
void *dispatcher(void *d) {
    // create an array to store all the news
    int counter = num_of_producers;
    while (counter != 0) {
        // RR the producers queues until they are all empty
        for (int i = 0; i < num_of_producers; i++){
            if (producers_queues[i] == NULL) {
                continue;
            } else {
                bool dequeue = false;
                // dequeue the news struct from the queue
                struct news *n = (struct news *) malloc(sizeof(struct news));
                dequeue = dequeueBoundedQueue(producers_queues[i], n);
                if (dequeue) {
                    if (strcmp(n->type, "DONE") == 0){
                        producers_queues[i] = NULL;
                        counter--;
                    } else {
                        bool enq = false;
                        while (!enq) {
                            // sort thr news by type and enqueue them in the co editors queues
                            if (strcmp(n->type, "SPORTS") == 0) {
                                enq = enqueueUnboundedQueue(sports_queue, n);
                            } else if (strcmp(n->type, "NEWS") == 0) {
                                enq = enqueueUnboundedQueue(news_queue, n);
                            } else if (strcmp(n->type, "WEATHER") == 0) {
                                enq = enqueueUnboundedQueue(weather_queue, n);
                            }
                        }
                    }
                } else {
                    // if dequeue failed-the current queue is empty, continue
                    continue;
                }
                // free n
                free(n);
            }
        }
    }
    // when all the producers are done, add a done news struct to each co editor queue
    struct news *done = (struct news *) malloc(sizeof(struct news));
    done->producer_id = -1;
    done->product_number = -1;
    done->type = "DONE";
    while (!enqueueUnboundedQueue(sports_queue, done));
    while (!enqueueUnboundedQueue(news_queue, done));
    while (!enqueueUnboundedQueue(weather_queue, done));
    // End of dispatcher thread
}


// This function is the co editor thread function
void *co_editor(void *news_type_queue) {
    // get the co editor queue pointer from c (one of the news types)
    struct UnboundedQueue *queue = (struct UnboundedQueue *) news_type_queue;
    bool flag = false;
    while(!flag){
        bool dequeue = false;
        while (!dequeue) {
            // dequeue the news from the relevant dispatcher-co-editor queue
            struct news *n = (struct news *) malloc(sizeof(struct news));
            dequeue = dequeueUnboundedQueue(queue, n);
            if (dequeue) {
                if (strcmp(n->type, "DONE") == 0){
                    flag = true;
                } else {
                    // Simulate The editing by waiting 0.1 seconds
                    usleep(100000);
                    bool enq = false;
                    while(!enq) {
                        // add the news struct to the screen manager queue
                        enq = enqueueBoundedQueue(co_editors_queue, n);
                    }
                }
            }
            // free n
            free(n);
        }
    }
    // when the co editor is done, add a done news struct to the screen manager queue
    struct news *done = (struct news *) malloc(sizeof(struct news));
    done->producer_id = -1;
    done->product_number = -1;
    done->type = "DONE";
    bool enq = false;
    while(!enq) {
        enq = enqueueBoundedQueue(co_editors_queue, done);
    }
    // End of co editor thread
}


// This function is the screen manager thread function
void *screen_manager(void *screen_manager_queue) {
    int down_counter = 3;
    int i = 0;
    // get the co editors combined queue from screen_manager_queue
    struct BoundedQueue *queue = (struct BoundedQueue *) screen_manager_queue;
    while(down_counter != 0) {
        bool dequeue = false;
        while (!dequeue) {
            // dequeue the news struct from the co-editors-screen manager queue
            struct news *n = (struct news *) malloc(sizeof(struct news));
            dequeue = dequeueBoundedQueue(queue, n);
            if (dequeue) {
                if (strcmp(n->type, "DONE") == 0){
                    down_counter--;
                } else {
                    // produce the news - print the news to the screen
                    printf("producer %d %s %d\n", n->producer_id, n->type, n->product_number);
                    i++;
                }
            }
            // free n
            free(n);
        }
    }
    // print the done message
    printf("DONE\n");
    // End of screen manager thread
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
    // todo replace 100
    pi = (struct producer_info *) malloc(100);
    // send the file name and the empty array to read file function to fill the struct array
    read_file(argv[1], pi);


    // create an array of pointers to bounded queues for each producer
    producers_queues = (struct BoundedQueue **) malloc(num_of_producers * sizeof(struct BoundedQueue *));
    // create the co editors queue to the screen manager
    co_editors_queue = create_Bqueue(co_editors_Q_size);


    //create a new producer thread for each producer to run the producer function
    for (int i = 0; i < num_of_producers; i++){
        // create a producer thread
        pthread_t producer_thread;
        pthread_create(&producer_thread, NULL, producer, &pi[i]);
    }

    // initialize the 3 co editors queues
    int size = 100;
    sports_queue = create_Uqueue(size);
    news_queue = create_Uqueue(size);
    weather_queue = create_Uqueue(size);

    // create a dispatcher thread
    pthread_t dispatcher_thread;
    pthread_create(&dispatcher_thread, NULL, dispatcher, NULL);

//    while (sports_queue == NULL || news_queue == NULL || weather_queue == NULL) {}

    // create 3 co-editor threads
    pthread_t co_editor_thread1;
    pthread_create(&co_editor_thread1, NULL, co_editor, sports_queue);
    pthread_t co_editor_thread2;
    pthread_create(&co_editor_thread2, NULL, co_editor, news_queue);
    pthread_t co_editor_thread3;
    pthread_create(&co_editor_thread3, NULL, co_editor, weather_queue);


    // create a screen manager thread
    pthread_t screen_manager_thread;
    pthread_create(&screen_manager_thread, NULL, screen_manager, co_editors_queue);

    // wait for all the threads to finish
    pthread_join(screen_manager_thread, NULL);

    //free pi array
    printf("the program has finished\n");
    free(pi);

    //free the queues
    free(sports_queue);
    free(news_queue);
    free(weather_queue);
    free(co_editors_queue);
    free(producers_queues);
    // End of main program
    return 0;
}
