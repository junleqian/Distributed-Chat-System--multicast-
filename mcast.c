#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#include "mp1.h"
#include <gnu/libc-version.h>
#include <glib.h>

typedef struct timestamp {
	int pid;
	int ts;
}timestamp;

GSList* holdback_queue;
GSList* deliverables;
timestamp* vector_timestamp;
int sequence_number;
GArray* cached_messages;

//GSList* piggyback_rqg;
//int piggyback_spg;

typedef struct mcast_message {
	int pid;
	//int piggyback_spg
	timestamp* timestamp;
	char* payload;
} mcast_message;


void multicast_init(void) {
	g_array_sized_new((gint)0, (gint)0, sizeof(char*), sizeof(char*)*mcast_num_members);
	unicast_init();
	holdback_queue = g_slist_alloc();
	deliverables = g_slist_alloc();
	//piggyback_rqg = g_slist_alloc();
	vector_timestamp = malloc(1+sizeof(int)*mcast_num_members);
	sequence_number = 0;
	int i;
	for(i = 0; i < mcast_num_members; i++) {
		vector_timestamp[i].pid = mcast_members[i];
		vector_timestamp[i].ts = 0;
	}
}

/* Basic multicast implementation */
void multicast(const char *message) {

	g_slist_append(cached_messages,message);

	int i;
	int size = sizeof(int) + (mcast_num_members*sizeof(timestamp)) + strlen(message)+1;
	mcast_message* m = malloc(size);
	char* manipulate = m;
	m->pid = my_id;
	sequence_number++;

	for(i = 0; i < mcast_num_members; i++) {
		if(vector_timestamp[i].pid = my_id) {
			vector_timestamp[i].ts += 1;
			break;
		}
	}

	void* mptr = manipulate+sizeof(int);
	memcpy(mptr,(void*)vector_timestamp,sizeof(timestamp)*mcast_num_members);

	mptr = manipulate+sizeof(int)+(mcast_num_members*sizeof(timestamp));
	strncpy(mptr, (void*)(message), strlen(message));
	((char*)mptr)[strlen(message)] = '\0';

	pthread_mutex_lock(&member_lock);
	for (i = 0; i < mcast_num_members; i++) {
		usend(mcast_members[i], (char*)m, sizeof(int)+(mcast_num_members*sizeof(timestamp)+strlen(message)+1));
	}
	pthread_mutex_unlock(&member_lock);
}

void checkAllDeliverables() {

	//FOREACH CHECK IF DELIVERABLE
	//IF SO SET VECTOR FOR J ++ and REMOVE FROM HBACK AND DELIVER
	// deliver source pfinal
}

int isDeliverable(void* message) {
	char* tmp = message;
}

void updateTimestamps() {
}

void receive(int source, const char *message, int len) {

	//if ( source == my_id) {
	//	return;
	//	}

	timestamp temp[mcast_num_members];
	char* t_ptr = message;
	t_ptr += sizeof(int);
	memcpy((void*)temp, (void*)t_ptr, sizeof(timestamp)*mcast_num_members);
	t_ptr += sizeof(timestamp)*mcast_num_members;

	updateTimestamps(temp);

	gpointer data = message;
	g_slist_append(holdback_queue,data);

	checkAllDeliverables();

	const char *pfinal = message + sizeof(int) + (mcast_num_members*sizeof(timestamp));
	deliver(source, pfinal);
}

void mcast_join(int member) {
	timestamp* new_ts = malloc(sizeof(timestamp)*mcast_num_members);
	int i;
	for(i=0; i < mcast_num_members-1; i++) {
		new_ts[i] = vector_timestamp[i];
	}
	new_ts[mcast_num_members-1].pid = member;
	new_ts[mcast_num_members-1].ts = 0;
	free(vector_timestamp);
	vector_timestamp = new_ts;
}
