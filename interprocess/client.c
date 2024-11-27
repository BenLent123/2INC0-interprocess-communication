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
#include "request.h"

char client2dealer[30] = "/c2d";
char dealer2worker1[30] = "/d2w";
char dealer2worker2[30] = "/d2w2";
char worker2dealer[30] = "/w2d";

static void rsleep (int t);


int main (int argc, char * argv[])
{
  struct mq_attr attr_c2d; //client to dealer ... and so on. c = client, d = dealer, w = worker
  struct mq_attr attr_d2w;
  struct mq_attr attr_d2w2;
  struct mq_attr attr_w2d;

  attr_c2d.mq_maxmsg  = MQ_MAX_MESSAGES;
  attr_c2d.mq_msgsize = size_req;
	mqd_t mq_c2d = mq_open (client2dealer_name, O_RDWR | O_EXCL, 0666, attr_c2d);
	
	
    // TODO:
    // (see message_queue_test() in interprocess_basic.c)
    //  * open the message queue (whose name is provided in the
    //    arguments)
    //  * repeatingly:
    //      - get the next job request 
    //      - send the request to the Req message queue
    //    until there are no more requests to send
    //  * close the message queue
    
    return (0);
}
