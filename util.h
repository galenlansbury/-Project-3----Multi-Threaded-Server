#ifndef _UTIL_H_
#define _UTIL_H_

struct request{
	pthread_mutex_t lock;
	pthread_cond_t not_full
    int seat_id;
    int user_id;
    int customer_priority;
    char* resource;
};

//int - the specific entry of the file descriptor that is taking the request
void parse_request(int, struct request*); //these simply goes through the buffer and sets the appropriate fields that correpsond to the request

void process_request(int, struct request*); //this calls the function into action. This is called each time a client makes a GET request

#endif
