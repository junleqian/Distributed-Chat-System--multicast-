#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#include "mp1.h"
#include <gnu/libc-version.h>
#include <glib.h>

GSList* holdback_queue;
GSList* deliverables;
int* vector_timestamp;
int piggyback_spg;
GSList* piggyback_rqg;

typedef struct mcast_message {
	int sequence_number;
	int* timestamp;
	char* payload;
} mcast_message;

int timestamp;

void multicast_init(void) {
    unicast_init();
		holdback_queue = g_slist_alloc();
		deliverables = g_slist_alloc();
		piggyback_rqg = g_slist_alloc();
		vector_timestamp = malloc(1+sizeof(int)*mcast_num_members);
		piggyback_spg = 0;
		int i;
		for (i = 0 ; i < mcast_num_members; i++) {
			vector_timestamp[i] = 0;
		}
}

/* Basic multicast implementation */
void multicast(const char *message) {
    int i;
		mcast_message* m = malloc(sizeof(struct mcast_message));
		m->payload = malloc(1+sizeof(message));
		m->sequence_number = my_id;
		m->timestamp = malloc(1+sizeof(vector_timestamp));
		m->payload = strcpy(message, m->payload);
		


    pthread_mutex_lock(&member_lock);
    for (i = 0; i < mcast_num_members; i++) {
        usend(mcast_members[i], message, strlen(message)+1);
    }
    pthread_mutex_unlock(&member_lock);
}

void receive(int source, const char *message, int len) {
    assert(message[len-1] == 0);
    deliver(source, message);
}

void mcast_join(int member) {

}
