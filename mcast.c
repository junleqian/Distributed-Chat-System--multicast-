#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#include "mp1.h"
#include <gnu/libc-version.h>
#include <glib.h>

// Structs

typedef struct timestamp {
	int pid;
	int ts;
}timestamp;

typedef struct mcast_message {
	int pid;
	timestamp* timestamp;
	char* payload;
} mcast_message;


// Global Variables

int *process_health; 					// List of failed and correct processes
GSList *holdback_queue;				// Queue for messages not yet ready to be delivered
GSList *deliverables;					// List of messages ready to be delivered
timestamp *vector_timestamp;	// Vector of timestamps to ensure causal ordering
int sequence_number;					// Sequence number for this process
GArray *cached_messages;			// List of all cached messages for resending

// Initializes data structures upon process creation
void multicast_init(void) {
	unicast_init();
	int i;

	// Initialize failure detection structure and spawn failure detector thread
	process_health = malloc(sizeof(int)*mcast_num_members);
	for (i = 0; i < mcast_num_members; i++) {
		process_health[i] = 1;
	}
	pthread_t fd;

	// Create array for cached messages
	g_array_sized_new((gint)0, (gint)0, sizeof(char*), sizeof(char*)*mcast_num_members);

	// Create holdback queue for unordered messages
	holdback_queue = g_slist_alloc();

	// Create list of deliverable messages (may not be required)
	deliverables = g_slist_alloc();

	// Initialize current process sequence number
	sequence_number = 0;

	// Create timestamp vector
	vector_timestamp = malloc(1+sizeof(int)*mcast_num_members);

	for(i = 0; i < mcast_num_members; i++) {
		vector_timestamp[i].pid = mcast_members[i];
		vector_timestamp[i].ts = 0;
	}
}


/* Basic multicast implementation */
void multicast(const char *message) {
	// Add this message to our cached list of messages
	g_slist_append(cached_messages,message);

	int size = sizeof(int) + (mcast_num_members*sizeof(timestamp)) + strlen(message)+1;
	mcast_message* m = malloc(size);
	char* manipulate = m; // Used for moving around the datastructure to remove headers
	m->pid = my_id;
	sequence_number++;

	int i;
	for(i = 0; i < mcast_num_members; i++) {
		if(vector_timestamp[i].pid = my_id) {
			vector_timestamp[i].ts += 1;
			break;
		}
	}

	void* mptr = manipulate+sizeof(int);

	// Copy over all timestamps
	memcpy(mptr,(void*)vector_timestamp,sizeof(timestamp)*mcast_num_members);

	mptr = manipulate+sizeof(int)+(mcast_num_members*sizeof(timestamp));

	// Copy over actual message
	strncpy(mptr, (void*)(message), strlen(message));
	((char*)mptr)[strlen(message)] = '\0';

	// Lock up during usend in case of race conditions
	pthread_mutex_lock(&member_lock);
	for (i = 0; i < mcast_num_members; i++) {
		usend(mcast_members[i], (char*)m, size);
	}
	pthread_mutex_unlock(&member_lock);
}


/* Iterates over holdback queue finding messages ready to be delivered */
void checkAllDeliverables() {

	//FOREACH CHECK IF DELIVERABLE
	//IF SO SET VECTOR FOR J ++ and REMOVE FROM HBACK AND DELIVER
	// deliver source pfinal
}

/* Returns a non-zero value if a given message is ready to be delivered */
int isDeliverable(void* message) {
	char* tmp = message;
}


void updateTimestamps(timestamp x[]) {
}


void receive(int source, const char *message, int len) {
	// Extract the timestamp for updating the current process timestamp
	timestamp temp[mcast_num_members];
	char* t_ptr = message;
	t_ptr += sizeof(int);
	memcpy((void*)temp, (void*)t_ptr, sizeof(timestamp)*mcast_num_members);
	t_ptr += sizeof(timestamp)*mcast_num_members;

	gpointer data = message;
	g_slist_append(holdback_queue,data);

	checkAllDeliverables();

	const char *pfinal = message + sizeof(int) + (mcast_num_members*sizeof(timestamp));
	for ( i = 0; i < mcast_num_members; i++) {
		debugprintf(" - %d - ", vector_timestamp[i].ts);
	}
	deliver(source, pfinal);
}

void mcast_join(int member) {
	timestamp* new_ts = malloc(sizeof(timestamp)*mcast_num_members);
	int* new_fd = malloc(sizeof(int)*mcast_num_members);
	int i;
	for(i=0; i < mcast_num_members-1; i++) {
		new_ts[i] = vector_timestamp[i];
		new_fd[i] = process_health[i];
	}
	new_ts[mcast_num_members-1].pid = member;
	new_ts[mcast_num_members-1].ts = 0;
	new_fd[mcast_num_members-1] = 1;
	free(process_health);
	free(vector_timestamp);
	process_health = new_fd;
	vector_timestamp = new_ts;
}
