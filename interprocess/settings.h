#ifndef _SETTINGS_H_
#define _SETTINGS_H_


// settings for interprocess communications
// (note: be sure that MQ_MAX_MESSAGES <= /proc/sys/fs/mqueue/msg_max (= 10 on most systems))
#define N_SERV1    	 1
#define N_SERV2    	 1
#define MQ_MAX_MESSAGES 	 1

#endif
