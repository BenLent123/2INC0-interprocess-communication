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

    mqd_t req_channel   = mq_open(argv[1], O_RDONLY);
    if(req_channel == (mqd_t)-1){
        perror("worker 2 - request channel opening failed");
        exit(EXIT_FAILURE);
    } else{perror("worker 2 - request channel opening succeeded");}

    mqd_t rsp_channel   = mq_open(argv[2], O_WRONLY);
    if(rsp_channel == (mqd_t)-1){
        perror("worker 2 - response channel opening failed");
        exit(EXIT_FAILURE);
    }else{perror("worker 2 - response channel opening succeeded");}
    
    while((1)){
        if(mq_receive(req_channel, (char*)&req, sizeof(S2_queue_T21),0) == -1){
            perror("worker 2 - receiving failed");
            mq_close(rsp_channel);
            mq_close(req_channel);
            exit(EXIT_FAILURE);
        }

        if(req.request_id != -1 && req.data != 0){
            rsleep(10000);
            rsp.result = service(req.data);
            rsp.request_id = req.request_id;
            if(mq_send(rsp_channel, (char*)&rsp, sizeof(Rsp_queue_T21),0) == -1){
            perror("worker 2 - sending failed");
            mq_close(rsp_channel);
            mq_close(req_channel);
            exit(EXIT_FAILURE);
            }
            printf("worker 2 sent work\n");
        } else{
            printf("kill signal received W2 \n");
            break;
        }
    }
    mq_close(rsp_channel);
    mq_close(req_channel); 
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
