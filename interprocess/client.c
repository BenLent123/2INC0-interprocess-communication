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
#include "settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>      // for perror()
#include <mqueue.h>     // for mq-stuff

#include "messages.h"
#include "request.h"
#include "settings.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Req_queue_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    mqd_t mq_c2d;
    struct mq_attr attr_c2d;
    req_queue_T21 req;
    int size_req = sizeof(req_queue_T21);

    attr_c2d.mq_flags = 0;
    attr_c2d.mq_maxmsg = MQ_MAX_MESSAGES;
    attr_c2d.mq_msgsize = size_req;
    attr_c2d.mq_curmsgs = 0;

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
            break;
        }
    }

    mq_close(mq_c2d);

    return 0;
}