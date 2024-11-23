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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>      // for perror()
#include <unistd.h>     // for getpid()
#include <mqueue.h>     // for mq-stuff
#include <time.h>       // for time()

#include "messages.h"
#include "service1.h"
static void rsleep (int t);


int main (int argc, char * argv[])
{
    Rsp_queue_X rsp;
    S1_queue_X req;
    mqd_t channel   = mq_open(argv[1], O_WRONLY);
    if(channel = (mqd_t)-1){
        perror("worker 1 - channel opening failed");
    }
    
    int result = mq_recieve(channel, (char*)&req, sizeof(S1_queue_X),0);
    if(result == -1){
        perror("worker 1 - recieveing failed");
    }
    rsleep(100);
    rsp.result = service(req.data);
    rsp.request_id = req.request_id;

    if(mq_send(channel, (char*)&rsp, sizeof(Rsp_queue_X)) == -1){
        perror("worker 1 - sending failed");
        mq_close(channel);
    }
    
    mq_close(channel);
    
    return(0);
}


// TODO:
    // (see message_queue_test() in interprocess_basic.c)
    //  * open the two message queues (whose names are provided in the
    //    arguments)
    //  * repeatedly:
    //      - read from the S1 message queue the new job to do
    //      - wait a random amount of time (e.g. rsleep(10000);)
    //      - do the job 
    //      - write the results to the Rsp message queue
    //    until there are no more tasks to do
    //  * close the message queues

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time
 * between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time (NULL) % getpid ());
        first_call = false;
    }
    usleep (random() % t);
}
