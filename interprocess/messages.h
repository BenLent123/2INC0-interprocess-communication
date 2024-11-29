/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 *
 * Nitin Singhal (1725963)
 * Daniel Tyukov (1819283)
 * Ben Lentschig (1824805)  
 *
 * Grading:
 * Your work will be evaluated based on the following criteria:
 * - Satisfaction of all the specifications
 * - Correctness of the program
 * - Coding style
 * - Report quality
 * - Deadlock analysis
 */

#ifndef MESSAGES_H
#define MESSAGES_H

// define the data structures for your messages here

typedef struct {
    int request_id;
    int service_id;
    int data;
}req_queue_T21;

typedef struct {
    int request_id;
    int data;
}S1_queue_T21;

typedef struct{
    int request_id;
    int data;
}S2_queue_T21;

typedef struct{
    int request_id;
    int result;
}Rsp_queue_T21;

#endif
