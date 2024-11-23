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

//Creates mutex for access to buffer
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

//Creates circular buffers for data transfers
  req_queue_T21 buffer_c2d[10]; int in_c2d = 0; int out_c2d = 0; int count1 = 0; int size_req = sizeof(req_queue_T21);
  S1_queue_T21 buffer_d2w[10]; int in_d2w = 0; int out_d2w = 0; int count2 = 0; int size_s1 = sizeof(S1_queue_T21);
  Rsp_queue_T21 buffer_w2d[10]; int in_w2d = 0; int out_w2d = 0; int count3 = 0; int size_rsp = sizeof(Rsp_queue_T21);

//Threading functions
void c2d_thread(mqd_t mq_c2d){
  
  while(true){
	  //Blocking read of client queue, data is stored in c2d buffer
	  //If statement is used to make sure that the buffer is not overloaded
	  if(count1<10) { //critical section of thread
		  pthread_mutex_lock(&m);
		  mq_receive(mg_c2d, buffer_c2d[in_c2d], size_req, NULL);
	  //Increments array variables
		  in_c2d = (in_c2d+1)%10;
	      ++count1;
	      pthread_mutex_unlock(&m);
	      } 
	  else sleep(0.00001);
	  //If the buffer is overloaded, the thread sleeps for 10 microseconds to allow the buffer to unload
  }
}

void forwarding_thread(){
	//This thread processes client data into the format needed by workers
	S1_queue_T21 rsp;
  while(true){
	  if(count1>0) { //critical section of thread
		  pthread_mutex_lock(&m);
		  //Extract data from buffer_c2d then put it in d2w and update buffer variables
		  rsp.request_id = buffer_c2d[out].request_id;
		  rsp.data = buffer_c2d[out].data;
		  
		  out_c2d = (out_c2d+1)%10;
	      --count1;
	      
	      buffer_d2w[in] = rsp;
	      
	      in_d2w = (in_d2w+1)%10;
	      ++count2;
	      
	      pthread_mutex_unlock(&m);
	      } 
	  else sleep(0.00001);
	  //If the buffer is empty, the thread sleeps for 10 microseconds to allow the buffer to fill
  }
}

void d2w_thread(mqd_t mq_d2w){
  
  while(true){
	  //Transfer data from client buffer to worker queue
	  //If statement is used to make sure that the buffer has data to transfer
	  if(count2>0) { //critical section of thread
		  pthread_mutex_lock(&m);
		  mq_send(mg_d2w, buffer_d2w[out_d2w], size_s1, NULL);
	  //Increments array variables
		  out_d2w = (out_d2w+1)%10;
	      --count2;
	      pthread_mutex_unlock(&m);
	      } 
	  else sleep(0.00001);
	  //If the buffer is empty, the thread sleeps for 10 microseconds to allow the buffer to fill
  }
}

void w2d_thread(mqd_t mq_w2d){  
  while(true){
	  pthread_mutex_lock(&m);
	  Rsp_queue_T21 rsp;
	  mq_receive(mg_w2d, rsp, size_rsp, NULL);
	  printf("RequestID: %d \n Result: %d", rsp.request_id,rsp.result);
	  pthread_mutex_unlock(&m);

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
  struct mq_attr attr_w2d;

  attr_c2d.mq_maxmsg  = 10;
  attr_c2d.mq_msgsize = sizeof (req_queue_T21);
  
  attr_d2w.mq_maxmsg  = 10;
  attr_d2w.mq_msgsize = sizeof (S1_queue_T21);
  
  attr_w2d.mq_maxmsg  = 10;
  attr_w2d.mq_msgsize = sizeof (Rsp_queue_T21);

  mqd_t mq_c2d = mq_open (client2dealer_name, O_RONLY | O_CREAT | O_EXCL, 0666, attr_c2d);
  mqd_t mq_d2w = mq_open (dealer2worker1_name, O_RONLY | O_CREAT | O_EXCL, 0666, attr_d2w);
  mqd_t mq_w2d = mq_open (worker2dealer_name, O_RONLY | O_CREAT | O_EXCL, 0666, attr_w2d);


  //Creates 4 threads. 3 for data transfer between processes, 
  //and 1 (the forwarding thread) for interthread communication
  //Each threading function uses mutexes to prevent race conditions
  pthread_t c2dthread; pthread_t fthread;   pthread_t d2wthread;   pthread_t w2dthread;  
  pthread_create(&c2dthread, NULL, c2d_thread(), mq_c2d);
  pthread_create(  &fthread, NULL, forwarding_thread(), NULL);
  pthread_create(&d2wthread, NULL, d2w_thread(), attr_d2w)
  pthread_create(&w2dthread, NULL, w2d_thread(), attr_w2d)

  //The following are children process to run the client and worker
  
  pid_t clientID = fork();
  pid_t workerID = fork();
  
  //error catching
  if (clientID < 0){perror("Client failed"); exit (1);}
  if (workerID < 0){perror("Worker failed"); exit (1);}

  //run the children processes. NOTE: FILL OUT FILES TO RUN
  if (clientID == 0){execlp ("./client", "client",client2dealer_name , NULL); perror ("Client execlp() failed");}
  if (workerID == 0){execlp ("./worker_s1", "worker_s1", dealer2worker1_name, worker2dealer_name NULL); perror ("worker execlp() failed");}
        
  // wait for the client to terminate
  waitpid (clientID, NULL, 0);   
  while((count1 != 0) | (count2 != 0) | (count3 != 0) ){} //waits until all buffers are emptied
  
  //The following section unlinks and closes all threads/queues
  if(mq_unlink(client2dealer_name) == 0) {printf("Client terminated and C2D queue marked for deletion");}
  else {perror("c2d queue not unlinked");}
  if(mq_close(client2dealer_name)==0){printf("C2D queue closed by parent thread");};
  else {perror("c2d queue not closed");}
  if(pthread_cancel(c2dthread)==0){printf("C2D thread terminated");};
  else {perror("c2d thread not terminated");}
 
  if(mq_unlink(dealer2worker1_name) == 0) {printf("D2W queue marked for deletion");}
  else {perror("D2W queue not unlinked");}
  if(mq_close(dealer2worker1_name)==0){printf("D2W queue closed by parent thread");};
  else {perror("D2W queue not closed");}
  if(pthread_cancel(d2wthread)==0){printf("D2W thread terminated");};
  else {perror("D2W thread not terminated");}
  
  if(mq_unlink(worker2dealer_name) == 0) {printf("W2D queue marked for deletion");}
  else {perror("W2D queue not unlinked");}
  if(mq_close(worker2dealer_name)==0){printf("W2D queue closed by parent thread");};
  else {perror("W2D queue not closed");}
  if(pthread_cancel(w2dthread)==0){printf("W2D thread terminated");};
  else {perror("W2D thread not terminated");}
  
  if(pthread_cancel(fthread)==0){printf("forwarding thread terminated");};
  else {perror("forwarding thread not terminated");}
  
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
