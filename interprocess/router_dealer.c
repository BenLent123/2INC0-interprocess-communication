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
#include <errno.h>    
#include <unistd.h>    // for execlp
#include <mqueue.h>    // for mq


#include "settings.h"  
#include "messages.h"

char client2dealer_name[30];
char dealer2worker1_name[30];
char dealer2worker2_name[30];
char worker2dealer_name[30];

//Creates request holding variables
//Request nextRequest;

//Creates circular buffers for data transfers
  req_queue_x buffer_c2d[10]; int in_c2d = 0; int out_c2d = 0; int count = 0; int size_req = sizeof(req_queue_x);
  S1_queue_X buffer_d2w[10];
  Rsp_queue_X buffer_w2d[10];

//Threading functions
void c2d_thread(struct mq_attr * attr){
  //Creates queue according to fed parameters to read client data
  mqd_t mq_c2d = mq_open (client2dealer_name, O_RONLY | O_CREAT | O_EXCL, 0666, attr);
  
  while(true){
	  //Blocking read of client queue, data is stored in c2d buffer
	  //If statement is used to make sure that the buffer is not overloaded
	  if(count<10) { //critical section of thread
		  pthread_mutex_lock(&m_c2d);
		  mq_receive (mg_c2d, buffer_c2d[in_c2d], size_req, NULL);
	  //Increments array variables
		  in_c2d = (in_c2d+1)%10;
	      ++count;
	      pthread_mutex_unlock(&m_c2d);
	      } 
	  else sleep(0.00001);
	  //If the buffer is overloaded, the thread sleeps for 10 microseconds to allow the buffer to unload
  }
}

int main (int argc, char * argv[])
{
  if (argc != 1)
  {
    fprintf (stderr, "%s: invalid arguments\n", argv[0]);
  }
  
  //Set queue parameters
  struct mq_attr attr_c2d; //client to dealer ... and so on. c = client, d = dealer, w = worker
  struct mq_attr attr_d2w;
  struct mq_attr attr_w2d;
  struct mq_attr attr_d2c; 

  attr_c2d.mq_maxmsg  = 10;
  attr_c2d.mq_msgsize = sizeof (req_queue_x);


  //Creates mutexes for each buffer
  pthread_mutex_t m_c2d = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t m_c2w = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t m_w2d = PTHREAD_MUTEX_INITIALIZER;


  //Creates 4 threads, one for each data flow path (c2d, d2w, w2d, d2c) so that all 4 queues can excecute simultaneously
  //Each threading function uses mutexes to prevent race conditions
  pthread_t c2dthread; //"forward" thread client2worker
  pthread_t bthread; //"backward" thread worker2 client
  pthread_create(&c2dthread, NULL, c2d_thread(), attr_c2d)
  
  
  
  // TODO:
    //  * create the message queues (see message_queue_test() in
    //    interprocess_basic.c)
    //  * create the child processes (see process_test() and
    //    message_queue_test())
    //  * read requests from the Req queue and transfer them to the workers
    //    with the Sx queues
    //  * read answers from workers in the Rep queue and print them
    //  * wait until the client has been stopped (see process_test())
    //  * clean up the message queues (see message_queue_test())

    // Important notice: make sure that the names of the message queues
    // contain your goup number (to ensure uniqueness during testing)
    // yeeer
  return (0);
}
