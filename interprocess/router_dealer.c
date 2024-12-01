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
#include <errno.h>    
#include <unistd.h>    // for execlp, fork, usleep
#include <mqueue.h>    // for mq
#include <signal.h>    // for kill
#include <time.h>

#include "settings.h"  
#include "messages.h"

const char * client2dealer_21 = "/client2dealer_21";
const char * dealer2worker1_21 = "/dealer2worker1_21";
const char * dealer2worker2_21 = "/dealer2worker2_21";
const char * worker2dealer_21 = "/worker2dealer_21";

// Some variables
int size_req = sizeof(req_queue_T21);
int size_s1 = sizeof(S1_queue_T21);
int size_s2 = sizeof(S2_queue_T21);
int size_res = sizeof(Rsp_queue_T21);

int main (int argc, char * argv[])
{
    if (argc != 1) 
    {
		fprintf (stderr, "%s: invalid arguments\n", argv[0]);
	}

    // Set queue parameters and open queues
    struct mq_attr attr_c2d; // client to dealer ... and so on. c = client, d = dealer, w = worker
    struct mq_attr attr_d2w;
    struct mq_attr attr_d2w2;
    struct mq_attr attr_w2d;

    attr_c2d.mq_flags = 0;
    attr_c2d.mq_maxmsg  = MQ_MAX_MESSAGES;
    attr_c2d.mq_msgsize = size_req;
    attr_c2d.mq_curmsgs = 0;

    attr_d2w.mq_flags = 0;
    attr_d2w.mq_maxmsg  = MQ_MAX_MESSAGES;
    attr_d2w.mq_msgsize = size_s1;
    attr_d2w.mq_curmsgs = 0;

    attr_d2w2.mq_flags = 0;
    attr_d2w2.mq_maxmsg  = MQ_MAX_MESSAGES;
    attr_d2w2.mq_msgsize = size_s2;
    attr_d2w2.mq_curmsgs = 0;

    attr_w2d.mq_flags = 0;
    attr_w2d.mq_maxmsg  = MQ_MAX_MESSAGES;
    attr_w2d.mq_msgsize = size_res;
    attr_w2d.mq_curmsgs = 0;

    mq_unlink(client2dealer_21);
    mq_unlink(dealer2worker1_21);
    mq_unlink(dealer2worker2_21);
    mq_unlink(worker2dealer_21);

    mqd_t mq_c2d = mq_open (client2dealer_21, O_RDONLY | O_CREAT | O_NONBLOCK, 0600, &attr_c2d);
    mqd_t mq_d2w = mq_open (dealer2worker1_21, O_WRONLY | O_CREAT | O_NONBLOCK, 0600, &attr_d2w);
    mqd_t mq_d2w2 = mq_open (dealer2worker2_21, O_WRONLY | O_CREAT | O_NONBLOCK, 0600, &attr_d2w2);
    mqd_t mq_w2d = mq_open (worker2dealer_21, O_RDONLY | O_CREAT | O_NONBLOCK, 0600, &attr_w2d);

    // Check if queues were opened correctly
    if ((mq_c2d == (mqd_t) -1) || (mq_d2w == (mqd_t) -1) || (mq_d2w2 == (mqd_t) -1) || (mq_w2d == (mqd_t) -1))
    {
		perror("Queue opening failed");
        exit(EXIT_FAILURE);
    }
    
    // Create client
    pid_t clientID = fork();
    if (clientID < 0)
    {	
        perror("Client fork failed");
        exit(EXIT_FAILURE);
    }   
    
    else if (clientID == 0)
    {
        execlp ("./client", "client", client2dealer_21, NULL);
        perror ("Client execlp() failed");
        exit(EXIT_FAILURE);
    }


    // Create worker_s1 processes
    for (int i = 0; i < N_SERV1; i++)
    {
        pid_t workerID = fork();
        if (workerID < 0)
        {
            perror("Worker_s1 fork failed");
            exit(EXIT_FAILURE);
        }
        else if (workerID == 0)
        {
            // In child process
            execlp ("./worker_s1", "worker_s1", dealer2worker1_21, worker2dealer_21, NULL);
            perror ("Worker_s1 execlp() failed");
            exit(EXIT_FAILURE);
        }
        // Parent continues
    }

    // Create worker_s2 processes
    for (int i = 0; i < N_SERV2; i++)
    {
        pid_t worker2ID = fork();
        if (worker2ID < 0)
        {
            perror("Worker_s2 fork failed");
            exit(EXIT_FAILURE);
        }
        else if (worker2ID == 0)
        {
            // In child process
            execlp ("./worker_s2", "worker_s2", dealer2worker2_21, worker2dealer_21, NULL);
            perror ("Worker_s2 execlp() failed");
            exit(EXIT_FAILURE);
        }
        // Parent continues
    }

	// variables for implementing data flow and control flow
    ssize_t bytes_read_req, bytes_read_rsp;
    int client_status = 1;
    int num_recieved = 0;
    int num_processed = 0;
	int num_dropped = 0;
	S1_queue_T21 s1_req;
    S2_queue_T21 s2_req;
    Rsp_queue_T21 res;
    int mq_sent_d;
    
    //Since worker queues are in non-blocking mode, the router can read
    //many client requests while waiting for the workers to finish
    //processing results. Client requests are stored in a circular loop
    int cap_req = 10;
	req_queue_T21 req[cap_req]; int in = 0; int out = 0; int count_req = 0;

	//This while loop implements the data routing functionality of 
	//the router
    while (1)
    {
        // Try to receive a message from client (if there is space in the request buffer)
        if(count_req<cap_req) 
        {
			bytes_read_req = mq_receive(mq_c2d, (char*) &req[in], size_req, NULL);
			if (bytes_read_req > 0)
			{
				num_recieved++;
				in=(in+1)%cap_req;
				count_req++;
			}
		}
		
		//If there are items in the request buffer, send them to workers
		if(count_req >0)
		{			
			// Send to worker_s1 queue
			if (req[out].service_id == 1)
			{
				s1_req.request_id = req[out].request_id;
				s1_req.data = req[out].data;
				
				mq_sent_d = mq_send(mq_d2w, (char*) &s1_req, size_s1, 0);
                if (mq_sent_d == -1 )
				{
					//perror("Router-Dealer: mq_send to worker_s1");
				} 
				
				else
				{
					out=(out+1)%cap_req;
					count_req--;
				}
					
                if((N_SERV1 == 0))
                {
					num_dropped++;
					count_req--;
				}
			}
				
			// Send to worker_s2 queue
			else if (req[out].service_id == 2)
            {
                s2_req.request_id = req[out].request_id;
                s2_req.data = req[out].data;
                
                mq_sent_d = mq_send(mq_d2w2, (char*) &s2_req, size_s2, 0);
                if (mq_sent_d == -1)
                {
                    // perror("Router-Dealer: mq_send to worker_s2");
                }
                
                else
                {
					out=(out+1)%cap_req;
					count_req--;
				}
					
                if((N_SERV2 == 0))
                {
					num_dropped++; 
					count_req--;
				}
            }
            //Unknown service ID
            else
            {
                fprintf(stderr, "Router-Dealer: Unknown service_id: %d\n", req[out].service_id);
            }
        }
        
        // Try to receive a message from workers
		bytes_read_rsp = mq_receive(mq_w2d, (char*) &res, size_res, NULL);
		if (bytes_read_rsp >= 0)
		{
			printf("%d -> %d\n", res.request_id, res.result);
			num_processed++;
		}
		
		else if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			perror("Router-Dealer: mq_receive from worker");
		}
		
		//Debugging statistics
		//fprintf(stderr,"Rec: %d, Proc: %d, count: %d, in: %d, out: %d \n",num_recieved,num_processed,count_req,in,out);
		
		// Check if client is still running
		if(client_status==1) 
		{
			waitpid(clientID, &client_status, WNOHANG);
		}
			
		else if ((num_processed+num_dropped)==num_recieved)
        {
			break;
        }
    }
   
    //Sends requests to workers to terminate themselves since they are no longer useful
	S1_queue_T21 kill_signal1; 
	kill_signal1.request_id = -1;
	kill_signal1.data = 0;
	S2_queue_T21 kill_signal2; 
	kill_signal2.request_id = -1;
	kill_signal2.data = 0;
  
	//Since every worker will terminate upon processing 1 kill_signal, sending N kill signals should terminate N workers
	fprintf(stderr,"sent kill signal to workers\n");
	for(int i =0; i<N_SERV1; i++){mq_send(mq_d2w, (char*) &kill_signal1, size_s1, 0); }
	for(int i =0; i<N_SERV2; i++){mq_send(mq_d2w2, (char*) &kill_signal2, size_s2, 0);}

    // Close the message queues
    mq_close(mq_c2d);
    mq_close(mq_d2w);
    mq_close(mq_d2w2);
    mq_close(mq_w2d);

    // Unlink the message queues
    mq_unlink(client2dealer_21);
    mq_unlink(dealer2worker1_21);
    mq_unlink(dealer2worker2_21);
    mq_unlink(worker2dealer_21);
    
    //Remove zombies
    int dump;
    for(int i =0; i< N_SERV1 + N_SERV2; i++){wait(&dump);}

    return 0;
}
