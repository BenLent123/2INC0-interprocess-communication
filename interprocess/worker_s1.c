/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 *
 * Ben Lentschig 1824805
 * Nitin Singhal
 * Daniel Tyukov 
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
#include "service1.h"
static void rsleep (int t);


int main (int argc, char * argv[])
{
   Rsp_queue_T21 rsp;
   S1_queue_T21 req;

    if (argc> 4){
        perror("worker 1 - to many arguments");
        exit(EXIT_FAILURE);
    }

    mqd_t req_channel   = mq_open(argv[1], O_RDONLY);
    if(req_channel == (mqd_t)-1){
        perror("worker 1 - request channel opening failed");
        mq_close(req_channel);
        exit(EXIT_FAILURE);
    }

    mqd_t rsp_channel   = mq_open(argv[2], O_WRONLY);
    if(rsp_channel == (mqd_t)-1){
        perror("worker 1 - response channel opening failed");
        mq_close(rsp_channel);
        exit(EXIT_FAILURE);
    }
    //pthread_lock(argv[3]);

        while(1){

        if(mq_receive(req_channel, (char*)&req, sizeof(S1_queue_T21),0) == -1){
            perror("worker 1 - recieveing failed");
            mq_close(rsp_channel);
            mq_close(req_channel);
            exit(EXIT_FAILURE);
        }

        if(req_request_id == -1 && req.data == 0){
            fprintf("worker 1 - termination signal");
            break;
        }

        rsleep(100);
        rsp.result = service(req.data);
        rsp.request_id = req.request_id;

        if(mq_send(rsp_channel, (char*)&rsp, sizeof(Rsp_queue_T21),0) == -1){
            perror("worker 1 - sending failed");
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
