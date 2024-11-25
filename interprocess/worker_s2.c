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
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>      // for perror()
#include <unistd.h>     // for getpid()
#include <mqueue.h>     // for mq-stuff
#include <time.h>       // for time()

#include "messages.h"
#include "service2.h"

static void rsleep (int t);

int main (int argc, char * argv[])
{    
    Rsp_queue_T21 rsp;
    S2_queue_T21 req;
    
    if (argc> 4){
        perror("worker 2 - to many arguments");
        exit(EXIT_FAILURE);
    }

    mqd_t req_channel   = mq_open(argv[1], O_RDONLY);
    if(req_channel == (mqd_t)-1){
        perror("worker 2 - request channel opening failed");
        mq_close(req_channel);
        exit(EXIT_FAILURE);
    }

    mqd_t rsp_channel   = mq_open(argv[2], O_WRONLY);
    if(rsp_channel == (mqd_t)-1){
        perror("worker 2 - response channel opening failed");
        mq_close(rsp_channel);
        exit(EXIT_FAILURE);
    }

    //pthread_lock(argv[3]);

    while(1){

        if(mq_receive(req_channel, (char*)&req, sizeof(S1_queue_T21),0) == -1){
            perror("worker 2 - recieveing failed");
            mq_close(rsp_channel);
            mq_close(req_channel);
            exit(EXIT_FAILURE);
        }

        if(req_request_id == -1 && req.data == 0){
            fprintf("worker 2 - termination signal");
            break;
        }

        rsleep(100);
        rsp.result = service(req.data);
        rsp.request_id = req.request_id;

        if(mq_send(rsp_channel, (char*)&rsp, sizeof(Rsp_queue_T21),0) == -1){
            perror("worker 2 - sending failed");
            mq_close(rsp_channel);
            mq_close(req_channel);
            exit(EXIT_FAILURE);
        }
    }
   
   mq_close(rsp_channel);
   mq_close(req_channel); 
   //pthread_unlock(argv[3]);
   return 0;   
}

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
