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
#include "service1.h"

static void rsleep (int t);

int main (int argc, char * argv[])
{
   Rsp_queue_T21 rsp;
   S1_queue_T21 req;
    // check amount of arguments
    if (argc < 3) {
        perror("worker 1 - argument amount failure");
        exit(EXIT_FAILURE);
    }

    // Open request channel (router to worker) (read only)
    mqd_t req_channel   = mq_open(argv[1], O_RDONLY); 
    if(req_channel == (mqd_t)-1){
        perror("worker 1 - request channel opening failed");
        exit(EXIT_FAILURE);
    }
    
    // Open response channel (worker to router) (write only)
    mqd_t rsp_channel   = mq_open(argv[2], O_WRONLY); 
    if(rsp_channel == (mqd_t)-1){
        perror("worker 1 - response channel opening failed");
        exit(EXIT_FAILURE);
    }

    int sent_state = 0;

    // while loop --> work done till termination signal
    while((1)){
        // check if something is recieved
        if(sent_state == 0 && mq_receive(req_channel, (char*)&req, sizeof(S1_queue_T21),0) == -1){
            perror("worker 1 - receiving failed\n");
            sent_state = 0;
            continue;
        }
        // do service 1 if termination is not called
        if(req.request_id != -1){
            rsleep(10000);
            rsp.result = service(req.data);
            rsp.request_id = req.request_id;
            if(mq_send(rsp_channel, (char*)&rsp, sizeof(Rsp_queue_T21),0) == -1){
            perror("worker 1 - sending failed");
            }
            sent_state = 1;
        }else{
            // exit the while loop
            break;
        }
    }
    // close all channels
    mq_close(rsp_channel);
    mq_close(req_channel);
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
