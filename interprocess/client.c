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
#include <errno.h>      // for perror()
#include <unistd.h>     // for getpid()
#include <mqueue.h>     // for mq-stuff
#include <time.h>       // for time()

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>      // for perror()
#include <mqueue.h>     // for mq-stuff

#include "messages.h"
#include "request.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Req_queue_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    mqd_t mq_c2d;
    req_queue_T21 req;
    int size_req = sizeof(req_queue_T21);

    mq_c2d = mq_open(argv[1], O_WRONLY);

    if (mq_c2d == (mqd_t)-1)
    {
        perror("Client: mq_open");
        exit(EXIT_FAILURE);
    }

    int jobID, data, serviceID;
    int ret;

    while ((ret = getNextRequest(&jobID, &data, &serviceID)) != NO_REQ)
    {
        if (ret != NO_ERR)
        {
            fprintf(stderr, "Client: Error getting next request\n");
            break;
        }

        req.request_id = jobID;
        req.service_id = serviceID;
        req.data = data;

        if (mq_send(mq_c2d, (char *)&req, size_req, 0) == -1)
        {
            perror("Client: mq_send");
            //break;
        }
    }
	usleep(1000);
    mq_close(mq_c2d);

    return 0;
}
