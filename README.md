# News Broadcasting System

## Program Structure  

**Producers**: Each producer creates news stories based on the configuration.
News stories are enqueued into individual bounded queues.
Upon completion, a "DONE" message is enqueued to signal the end.

**Dispatcher**: The dispatcher scans producer queues using a Round Robin algorithm.
News stories are dequeued from producer queues and sorted into respective co-editor queues.
Upon completion, the dispatcher sends "DONE" messages to each co-editor queue.

**Co-Editors**: Each co-editor dequeues news stories from the dispatcher's queue.
Simulates editing by blocking for 0.1 seconds.
Edited stories are enqueued into the shared co-editor to the screen manager queue.
Upon receiving a "DONE" message, it immediately passes it to the screen manager.

**Screen Manager**: Dequeues edited news stories from the co-editor to the screen manager queue.
Prints news stories to the console.
Displays a "DONE" statement after receiving three "DONE" messages.

**Bounded Buffer**: Implements thread-safe bounded buffer operations.
Two types: BoundedQueue (for producers) and BoundedQueue (for co-editors to screen manager).


## Thread Descriptions

### Producer Threads
Create news stories and enqueue them into their private bounded queues.
Signal completion by enqueuing a "DONE" message.
### Dispatcher Thread
Round Robin scan producer queues.
Sort and enqueue news stories into co-editor queues.
Signal completion by enqueuing "DONE" messages.
### Co-Editor Threads
Dequeue news stories from the dispatcher.
Simulate editing and enqueue edited stories to the screen manager.
Signal completion by enqueuing "DONE" messages.
### Screen Manager Thread
Dequeue edited news stories and print them to the console.
Display "DONE" statement after receiving three "DONE" messages.


## System Design:

<img width="380" alt="image" src="https://github.com/noatal345/Producer_consumer_problem/assets/72741540/2cb00914-08c3-4d05-bd5c-175c08a202f5">

## Configuration File
The configuration file specifies the number of producers, the number of products each producer creates, and the size of their respective queues. Additionally, it includes the size of the co-editor's shared queue.

Example of a Configuration file:  

1  
30  
5  

2  
25  
3  

3  
16  
30  

17  



## How to Use
*Compile the program using the provided makefile:*  
### make

*Run the program with a configuration file:*  
### ./ex3.out config.txt
