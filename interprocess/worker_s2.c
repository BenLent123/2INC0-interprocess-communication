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
	
	// check amount of arguments
    if (argc < 3) {
        perror("worker 2 - argument amount failure");
        exit(EXIT_FAILURE);
    }
	
    // Open request channel (router to worker) (read only)
    mqd_t req_channel   = mq_open(argv[1], O_RDONLY); 
    if(req_channel == (mqd_t)-1){
        perror("worker 2 - request channel opening failed");
        exit(EXIT_FAILURE);
    }
    
    // Open response channel (worker to router) (write only)
    mqd_t rsp_channel   = mq_open(argv[2], O_WRONLY); 
    if(rsp_channel == (mqd_t)-1){
        perror("worker 2 - response channel opening failed");
        exit(EXIT_FAILURE);
    }
    
	int mq_received = 0;
	int mq_sent = 0;
	
    while((1)){
		//The first if is so that an attempt to receive is only made if 
		//something was sent previous cycle, or this is the initial loop
		//This is to prevent jobs from being overwritten
		if(mq_sent==0) {
			mq_received = mq_receive(req_channel, (char*)&req, sizeof(S2_queue_T21),0);
		}
        else if(mq_received == -1){
            perror("worker 2 - receiving failed\n");
            mq_sent = 0; 
	        continue;
            //Try recieving again in next loop
        }
        
        if(req.request_id != -1){
            rsleep(10000);
            rsp.result = service(req.data);
            rsp.request_id = req.request_id;
            mq_sent = mq_send(rsp_channel, (char*)&rsp, sizeof(Rsp_queue_T21),0);
            if(mq_sent == -1){
			    perror("worker 2 - sending failed");
			}
        }else{
             //break out of while loop
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
