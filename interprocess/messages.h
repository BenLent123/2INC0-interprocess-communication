/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 *
 * STUDENT_NAME_1 (STUDENT_NR_1)
 * STUDENT_NAME_2 (STUDENT_NR_2)
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
}req_queue_x;

typedef struct {
    int request_id;
    int data;
}S1_queue_X;

typedef struct{
    int request_id;
    int data;
}S2_queue_X;

typedef struct{
    int request_id;
    int result;
}Rsp_queue_X;

#endif
