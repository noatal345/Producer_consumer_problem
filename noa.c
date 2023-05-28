//
// Created by USER on 28/05/2023.
//
    // create a queue
//struct BoundedQueue *queue = create_queue(3);
//// create a news struct
//struct news *n = (struct news *) malloc(sizeof(struct news));
//n->producer_id = 1;
//n->product_number = 1;
//n->type = "SPORTS";
//struct news *a = (struct news *) malloc(sizeof(struct news));
//a->producer_id = 1;
//a->product_number = 2;
//a->type = "SPORTS";
//struct news *b = (struct news *) malloc(sizeof(struct news));
//b->producer_id = 1;
//b->product_number = 3;
//b->type = "SPORTS";
//struct news *c = (struct news *) malloc(sizeof(struct news));
//c->producer_id = 1;
//c->product_number = 4;
//c->type = "SPORTS";
//// enqueue the news struct
//bool enqueue = enqueueBoundedQueue(queue, n);
//if (!enqueue){
//printf("enqueue failed\n");
//}
//// enqueue the news struct
//enqueue = enqueueBoundedQueue(queue, a);
//if (!enqueue){
//printf("enqueue failed\n");
//}
//// enqueue the news struct
//enqueue = enqueueBoundedQueue(queue, b);
//if (!enqueue){
//printf("enqueue failed\n");
//}
//// enqueue the news struct
//enqueue = enqueueBoundedQueue(queue, c);
//if (!enqueue){
//printf("enqueue failed\n");
//}
//// dequeue the news struct
//struct news *n2 = (struct news *) malloc(sizeof(struct news));
//bool dequeue = dequeueBoundedQueue(queue, n2);
//if (!dequeue){
//printf("dequeue failed\n");
//} else {
//printf("product_num=%d\n", n2->product_number);
//}
//// enqueue the news struct
//enqueue = enqueueBoundedQueue(queue, c);
//if (!enqueue){
//printf("enqueue failed\n");
//}
//dequeue = dequeueBoundedQueue(queue, n2);
//if (!dequeue){
//printf("dequeue failed\n");
//} else {
//printf("product_num=%d\n", n2->product_number);
//}
//dequeue = dequeueBoundedQueue(queue, n2);
//if (!dequeue){
//printf("dequeue failed\n");
//} else {
//printf("product_num=%d\n", n2->product_number);
//}
//dequeue = dequeueBoundedQueue(queue, n2);
//if (!dequeue){
//printf("dequeue failed\n");
//} else {
//printf("product_num=%d\n", n2->product_number);
//}
//printf("head=%d, tail=%d\n", queue->head_index, queue->tail_index);
//printf("is q full=%d\n", queue->is_full);
//dequeue = dequeueBoundedQueue(queue, n2);
//if (!dequeue){
//printf("dequeue failed\n");
//} else {
//printf("product_num=%d\n", n2->product_number);
//}
//enqueue = enqueueBoundedQueue(queue, a);
//if (!enqueue){
//printf("enqueue failed\n");
//}
//dequeue = dequeueBoundedQueue(queue, n2);
//if (!dequeue){
//printf("dequeue failed\n");
//} else {
//printf("product_num=%d\n", n2->product_number);
//}