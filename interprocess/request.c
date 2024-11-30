/* 
 * Operating Systems (2INCO) Practical Assignment
 * Interprocess Communication
 *
 * Contains functions that are used by the clients
 *
 */

#include "request.h"

// Array of requests
const Request requests[] = {{1,1,1},{2,1,1},{3,1,1},{4,1,1},{5,1,1},{6,1,1},{7,1,1},{8,1,1},{9,1,1},{10,1,1},{11,1,1},{12,1,1},{13,1,1},{14,1,1},{15,1,1},{16,2,2},{17,2,2},{18,2,2},{19,2,2},{20,2,2},{21,2,2},{22,2,2},{23,2,2},{24,2,2},{25,2,2},{26,2,2},{27,2,2},{28,2,2},{29,2,2},{30,2,2}};

// Places the information of the next request in the parameters sent by reference.
// Returns NO_REQ if there is no request to make.
// Returns NO_ERR otherwise.
int getNextRequest(int* jobID, int* data, int* serviceID) {
	static int i = 0;
	static int N_REQUESTS = sizeof(requests) / sizeof(Request);

	if (i >= N_REQUESTS) 
		return NO_REQ;

	*jobID = requests[i].job;
	*data = requests[i].data;
	*serviceID = requests[i].service;		
	++i;
	return NO_ERR;
		
}
