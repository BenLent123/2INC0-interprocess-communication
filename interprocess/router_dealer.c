/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 *
 * Nitin Singhal (1725963)
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

const char * client2dealer_name = "/c2d";
const char * dealer2worker1_name = "/d2w";
const char * dealer2worker2_name = "/d2w2";
const char * worker2dealer_name = "/w2d";

//Some variables
int size_req = sizeof(req_queue_T21);
int size_s1 = sizeof(S1_queue_T21);
int size_s12 = sizeof(S2_queue_T21);
int size_res = sizeof(Rsp_queue_T21);

int main (int argc, char * argv[])
{
  if (argc != 1)
  {
    fprintf (stderr, "%s: invalid arguments\n", argv[0]);
  }
  
  req_queue_T21 req;
  S1_queue_T21 rsp;
  S2_queue_T21 rsp2;
  Rsp_queue_T21 res;
  
  //Set queue parameters and open queues
  struct mq_attr attr_c2d; //client to dealer ... and so on. c = client, d = dealer, w = worker
  struct mq_attr attr_d2w;
  struct mq_attr attr_d2w2;
  struct mq_attr attr_w2d;

  attr_c2d.mq_maxmsg  = MQ_MAX_MESSAGES;
  attr_c2d.mq_msgsize = size_req;
  
  attr_d2w.mq_maxmsg  = MQ_MAX_MESSAGES;
  attr_d2w.mq_msgsize = size_s1;
  
  attr_d2w2.mq_maxmsg  = MQ_MAX_MESSAGES;
  attr_d2w2.mq_msgsize = size_s12;
  
  attr_w2d.mq_maxmsg  = MQ_MAX_MESSAGES;
  attr_w2d.mq_msgsize = size_res;

  mq_unlink(client2dealer_name);
  mq_unlink(dealer2worker1_name);
  mq_unlink(dealer2worker2_name);
  mq_unlink(worker2dealer_name);

  mqd_t mq_c2d = mq_open (client2dealer_name, O_RDWR | O_CREAT , 0600, &attr_c2d);
  mqd_t mq_d2w = mq_open (dealer2worker1_name, O_RDWR | O_CREAT, 0600, &attr_d2w);
  mqd_t mq_d2w2 = mq_open (dealer2worker2_name, O_RDWR | O_CREAT, 0600, &attr_d2w2);
  mqd_t mq_w2d = mq_open (worker2dealer_name, O_RDWR | O_CREAT , 0600, &attr_w2d);

  mqd_t queues[4] = {mq_c2d,mq_d2w,mq_d2w2,mq_w2d};

 //Check if queues were opened correctly
 
  if((mq_c2d == (mqd_t) -1)|| (mq_d2w == (mqd_t) -1) || (mq_d2w2 == (mqd_t) -1) || (mq_w2d == (mqd_t) -1))
   {perror("queue opening failed\n"); exit(1);}
   else fprintf(stderr,"Queues opened successfully\n");

  //The following are children process to run the client and worker
  //A loop goes through forking to create enough workers
  pid_t clientID; int c_stat = 0; pid_t cpid;
  pid_t workerID;
  pid_t worker2ID;
  
  //Create client
  clientID = fork();
  if (clientID < 0){perror("Client failed"); exit (1);}
  else if (clientID == 0)
	{execlp ("./client", "client",client2dealer_name , NULL); perror ("Client execlp() failed"); exit (1);}
  //If this part of the code is reached, the code is in the parent/router_dealer

  while(true){
		mq_receive(mq_c2d, (char*) &req, size_req, NULL); 
		if(req.service_id == 1){
			rsp.request_id = req.request_id;
			rsp.data = req.data;
			mq_send(mq_d2w, (char*) &rsp, size_s1, 0);
			workerID = fork();
			if (workerID < 0){perror("Worker failed"); exit (1);}
			if (workerID == 0)
				{execlp ("./worker_s1", "worker_s1", dealer2worker1_name, worker2dealer_name, NULL); perror ("worker execlp() failed"); exit (1);}
			}
		else if (req.service_id == 2){
			rsp2.request_id = req.request_id;
			rsp2.data = req.data;
			mq_send(mq_d2w2, (char*) &rsp2, size_s12, 0);
			worker2ID = fork();
			if (worker2ID < 0){perror("Worker failed"); exit (1);}
			if (worker2ID == 0)
				{execlp ("./worker_s2", "worker_s2", dealer2worker2_name, worker2dealer_name, NULL); perror ("worker execlp() failed"); exit (1);}
			}
			mq_receive(mq_w2d, (char*) &res, size_res, NULL);
			printf("RequestID: %d \n Result: %d", res.request_id, res.result);
			cpid = waitpid(clientID, &c_stat, WNOHANG);
			if(cpid == 0) {break;}
		}
		  
  //Get queue status
  mq_getattr(mq_c2d, &attr_c2d);
  mq_getattr(mq_d2w, &attr_d2w);
  mq_getattr(mq_d2w2, &attr_d2w2);
  mq_getattr(mq_w2d, &attr_w2d);

  while((attr_c2d.mq_curmsgs != 0) || (attr_d2w.mq_curmsgs != 0) || (attr_d2w2.mq_curmsgs != 0) || (attr_w2d.mq_curmsgs != 0)){
	  sleep(0.00001); //wait for queues have emptied
	  mq_getattr(mq_c2d, &attr_c2d);
	  mq_getattr(mq_d2w, &attr_d2w);
	  mq_getattr(mq_d2w2, &attr_d2w2);
	  mq_getattr(mq_w2d, &attr_w2d);}
  
  //The following section unlinks and closes all threads/queues
  if(mq_unlink(client2dealer_name) != 0) {perror("c2d queue not unlinked");}
  if(mq_close(mq_c2d)!=0)    {perror("c2d queue not closed");}
 
  if(mq_unlink(dealer2worker1_name) != 0){perror("D2W queue not unlinked");}
  if(mq_close(mq_d2w)!=0)   {perror("D2W queue not closed");}

  if(mq_unlink(dealer2worker2_name) != 0){perror("D2W2 queue not unlinked");}
  if(mq_close(mq_d2w2)!=0)   {perror("D2W2 queue not closed");}
  
  if(mq_unlink(worker2dealer_name) != 0) {perror("W2D queue not unlinked");}
  if(mq_close(mq_w2d)!=0)    {perror("W2D queue not closed");}
    
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
