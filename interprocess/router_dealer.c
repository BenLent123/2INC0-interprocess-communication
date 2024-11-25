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

char client2dealer_name[30];
char dealer2worker1_name[30];
char dealer2worker2_name[30];
char worker2dealer_name[30];

//Creates mutex for access to buffer
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

//Some variables
int size_req = sizeof(req_queue_T21);
int size_s1 = sizeof(S1_queue_T21);
int size_s12 = sizeof(S2_queue_T21);
int size_rsp = sizeof(Rsp_queue_T21);

//Threading functions. The threads don't have error checking to keep them lightweight. This could be changed.
void forwarding_thread(){
	//Description:
	//It extracts data from the c2d buffer, processes it to the format needed by workers, then puts it in d2w buffers
	req_queue_T21 req;
	S1_queue_T21 rsp;
	S2_queue_T21 rsp2;
	int out_c2d = 0;
	int in_d2w = 0;
	int in_d2w2 = 0;
	while(true){
		pthread_mutex_lock(&m); //Critical section start
		mq_receive(mq_c2d, (char*) &req, size_req, NULL); 
		if(req.service_id == 1){
			rsp.request_id = req.request_id;
			rsp.data = req.data;
			mq_send(mg_d2w, (char*) &rsp, size_s1, NULL);
			pthread_mutex_unlock(&m); //Critical section end (on this branch)
			}
		else if (req.service_id == 2){
			rsp2.request_id = req.request_id;
			rsp2.data = req.data;
			mq_send(mg_d2w2, (char*)
			&rsp2, size_s12, NULL);
			pthread_mutex_unlock(&m); //Critical section end (on this branch)
			}
		} 
	}

void w2d_thread(mqd_t mq_w2d){  
	//Description:
	//This thread prints the response of the workers on stdout
	Rsp_queue_T21 rsp;
	while(true){
		pthread_mutex_lock(&m); //Critical section start
		mq_receive(mg_w2d, (char*) &rsp, size_rsp, NULL);
		pthread_mutex_unlock(&m); //Critical section end
		printf("RequestID: %d \n Result: %d", rsp.request_id,rsp.result);

	}
}

int main (int argc, char * argv[])
{
  if (argc != 1)
  {
    fprintf (stderr, "%s: invalid arguments\n", argv[0]);
  }
  
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
  attr_w2d.mq_msgsize = size_rsp;

  mqd_t mq_c2d = mq_open (client2dealer_name, O_RONLY | O_CREAT | O_EXCL, 0666, attr_c2d);
  mqd_t mq_d2w = mq_open (dealer2worker1_name, O_RONLY | O_CREAT | O_EXCL, 0666, attr_d2w);
  mqd_t mq_d2w2 = mq_open (dealer2worker2_name, O_RONLY | O_CREAT | O_EXCL, 0666, attr_d2w2);
  mqd_t mq_w2d = mq_open (worker2dealer_name, O_RONLY | O_CREAT | O_EXCL, 0666, attr_w2d);

 //Check if queues were opened correctly
 
  if((mq_c2d == (mqd_t) –1)|| (mq_d2w == (mqd_t) –1) || (mq_d2w2 == (mqd_t) –1) || (mq_w2d == (mqd_t) –1))
   {perror("queue opening failed"); exit(1);}
   else printf("Queues opened successfully");


  //Creates 2 threads. 1 for data transfer from client to worker, and 1 to read results. The parent
  // is on duty to watch if the client terminates or not
  //Each threading function uses mutexes to prevent race conditions
   pthread_t fthread;    pthread_t w2dthread;  
  if (
	(pthread_create(  &fthread, NULL, forwarding_thread(), NULL)  == 0) &&
	(pthread_create(&w2dthread, NULL, w2d_thread(), attr_w2d)  == 0) 
	){
	  fprintf("Threads created successfully");}
   else {fprintf("Threads not created successfully"); exit(1);}
  
  //The following are children process to run the client and worker
  //A loop goes through forking to create enough workers
  pid_t clientID;
  pid_t workerID[N_SERV1];
  pid_t worker2ID[N_SERV2];
  
  //Create client
  clientID = fork();
  if (clientID < 0){perror("Client failed"); exit (1);}
  else if (clientID == 0)
	{execlp ("./client", "client",client2dealer_name , &m, NULL); perror ("Client execlp() failed"); exit (1);}
  //If this part of the code is reached, the code is in the parent/router_dealer
  
  for(int i =0; i<N_SERV1; i++){
	  workerID[i] = fork();
	  if (workerID[i] < 0){perror("Worker failed"); exit (1);}
	  if (workerID[i] == 0)
		{execlp ("./worker_s1", "worker_s1", dealer2worker1_name, worker2dealer_name, &m, NULL); perror ("worker execlp() failed"); exit (1);}
	  //Only in the parent process will the code reach this point to loop around again
  }
  
  for(int i =0; i<N_SERV2; i++){
	  worker2ID[i] = fork();
	  if (worker2ID[i] < 0){perror("Worker failed"); exit (1);}
	  if (worker2ID[i] == 0)
		{execlp ("./worker_s2", "worker_s2", dealer2worker2_name, worker2dealer_name, &m, NULL); perror ("worker execlp() failed"); exit (1);}
 
	  //Only in the parent process will the code reach this point to loop around again
  }
	 
  // wait for the client to terminate
  waitpid (clientID, NULL, 0);   
  
  //Get queue status
  mq_getattr(mq_c2d, &attr_c2d);
  mq_getattr(mq_d2w, &attr_d2w);
  mq_getattr(mq_d2w2, &attr_d2w2);
  mq_getattr(mq_w2d, &attr_w2d);

  while((attr_c2d.mq_curmsgs != 0) || (attr_d2w.mq_curmsgs != 0) || (attr_d2w2.mq_curmsgs != 0) || (attr_w2d.mq_curmsgs != 0)){
	  sleep(0.00001); //wait for threads to make progress then check if queues have emptied
	  mq_getattr(mq_c2d, &attr_c2d);
	  mq_getattr(mq_d2w, &attr_d2w);
	  mq_getattr(mq_d2w2, &attr_d2w2);
	  mq_getattr(mq_w2d, &attr_w2d);}
  
  //while((count1 != 0) | (count2 != 0) | (count3 != 0) ){} //waits until all buffers are emptied
  
  //Sends requests to workers to terminate themselves since they are no longer useful
  S1_queue_T21 kill_signal1; 
  kill_signal1.request_id = -1;
  kill_signal1.data = 0;
  S2_queue_T21 kill_signal2; 
  kill_signal2.request_id = -1;
  kill_signal2.data = 0;
  
  //Since every worker will terminate upon processing 1 kill_signal, sending N kill signals should terminate N workers
  pthread_mutex_lock(&m); //Critical section start
  for(int i =0; i<N_SERV1; i++){mq_send(mg_d2w, &kill_signal1, size_s1, NULL);}
  for(int i =0; i<N_SERV2; i++){mq_send(mg_d2w2, &kill_signal2, size_s12, NULL);}
  pthread_mutex_unlock(&m); //Critical section end
  
  //The following section unlinks and closes all threads/queues
  if(mq_unlink(client2dealer_name) == 0) {fprintf("Client terminated and C2D queue marked for deletion");}
  else {perror("c2d queue not unlinked");}
  if(mq_close(client2dealer_name)==0){fprintf("C2D queue closed by parent thread");};
  else {perror("c2d queue not closed");}
 
  if(mq_unlink(dealer2worker1_name) == 0) {fprintf("D2W queue marked for deletion");}
  else {perror("D2W queue not unlinked");}
  if(mq_close(dealer2worker1_name)==0){fprintf("D2W queue closed by parent thread");};
  else {perror("D2W queue not closed");}

  if(mq_unlink(dealer2worker2_name) == 0) {fprintf("D2W2 queue marked for deletion");}
  else {perror("D2W2 queue not unlinked");}
  if(mq_close(dealer2worker2_name)==0){fprintf("D2W2 queue closed by parent thread");};
  else {perror("D2W2 queue not closed");}
  
  if(mq_unlink(worker2dealer_name) == 0) {fprintf("W2D queue marked for deletion");}
  else {perror("W2D queue not unlinked");}
  if(mq_close(worker2dealer_name)==0){fprintf("W2D queue closed by parent thread");};
  else {perror("W2D queue not closed");}
  if(pthread_cancel(w2dthread)==0){fprintf("W2D thread terminated");};
  else {perror("W2D thread not terminated");}
  
  if(pthread_cancel(fthread)==0){fprintf("forwarding thread terminated");};
  else {perror("forwarding thread not terminated");}
  
  pthread_mutex_destroy(&m);
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
