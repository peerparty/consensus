#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "citcomm.h"

// id of this node/citizen - JBG
unsigned long int node_id = 0;
// last promise id/count/msg_id - JBG
unsigned long int promise = 0;
// last commit to id/count/msg_id - JBG
unsigned long int commit = 0;

// How many promises (participants) should we get commits from - JBG
unsigned long int promise_count = 0;
// Count our commits - JBG
unsigned long int commit_count = 0;

// listener thread - JBG
pthread_t t;

/*
unsigned long int convert_ids(struct Payload payload) {
  char payload_num_str[10];
  sprintf(payload_num_str, "%lu%lu", 
          payload.msg_id,
          payload.node_id);

  char* end;
  return strtoul(payload_num_str, &end, 10);
}
*/

void response_wait(unsigned long int msg_id,
                   void(*callback)(unsigned long int)) {
  struct timespec abs_time;
  pthread_t t_rw;
  pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t done = PTHREAD_COND_INITIALIZER;
  pthread_mutex_lock(&lock);

  clock_gettime(CLOCK_REALTIME, &abs_time);
  abs_time.tv_sec += 10;
  if(pthread_cond_timedwait(&done, &lock, &abs_time)) callback(msg_id);
  else printf("TIMEOUT FAIL!\n");
}

void send_promise(unsigned long int dest_id,
                  unsigned long int msg_id) {

  printf("Promise: %lu\n", msg_id);

  struct Payload payload;
  payload.cmd = PROMISE;
  payload.node_id = node_id;
  payload.msg_id = msg_id;
  payload.dest_id = dest_id;
  client_send(payload);
}

void send_commit(unsigned long int dest_id,
                 unsigned long int msg_id) {

  printf("Commit: %lu\n", msg_id);

  struct Payload payload;
  payload.cmd = COMMIT;
  payload.node_id = node_id;
  payload.msg_id = msg_id;
  payload.dest_id = dest_id;
  client_send(payload);
}

void handle_commits(unsigned long int msg_id) {
  if(promise_count == commit_count) {
    printf("All citizens have committed.\n");
  }
}

void send_commit_req(unsigned long int msg_id) {
  commit_count = 0;
  printf("CALLBACK: %lu with %lu responses\n", msg_id, promise_count);

  struct Payload payload;
  payload.cmd = COMMIT_REQ;
  payload.node_id = node_id;
  payload.msg_id = msg_id;
  client_send(payload);

  // Wait for citizens to commit, then callback - JBG 
  void (*callback)(unsigned long int msg_id);
  callback = &handle_commits;
  response_wait(msg_id, callback);
}

void send_promise_req(unsigned long int msg_id) {
  // If we are requesting the floor, reset promise count - JBG
  promise_count = 0;

  struct Payload payload;
  payload.cmd = PROMISE_REQ;
  payload.node_id = node_id;
  payload.msg_id = msg_id;
  client_send(payload);

  // Wait for citizens to promise participation, then callback - JBG 
  void (*callback)(unsigned long int msg_id);
  callback = &send_commit_req;
  response_wait(msg_id, callback);

}

void handle_payload(struct Payload payload) {

  // Don't handle my own messages - JBG
  if(payload.node_id == node_id) return;

  /*
  printf("Payload: %d %lu%lu\n",
         payload.cmd,
         payload.msg_id,
         payload.node_id);

  printf("CHECKING... %d %d\n",
         payload.cmd == PROMISE,
         payload.dest_id == node_id);
  */

  //unsigned long int n = convert_ids(payload);

  if(payload.cmd == PROMISE_REQ && payload.msg_id > promise) {
    promise = payload.msg_id;
    send_promise(payload.node_id, payload.msg_id);
  } else if(payload.cmd == PROMISE &&
            payload.dest_id == node_id) {
    printf("Promise from: %lu\n", payload.node_id);
    promise_count++;
  } else if(payload.cmd == COMMIT_REQ && payload.msg_id == promise) {
    send_commit(payload.node_id, payload.msg_id);
  } else if(payload.cmd == COMMIT) {
    printf("Commit from: %lu\n", payload.node_id);
    commit_count++;
  } else {
    printf("Ignored: %d %d\n",
           payload.cmd == PROMISE_REQ,
           payload.msg_id > promise);
  }
}

void start_listen() {
  void (*handler)(struct Payload);
  handler = &handle_payload;

  if(pthread_create(&t, NULL, server_listen, handler)) {
    fprintf(stderr, "Error creating thread\n");
    exit(1);
  }
}

int main(int argc, char **argv) {

  char* end;

  if(argc < 1 || argc > 3) {
    printf("USAGE: citizen node-id msg-id\n");
    return 1;
  } else if(argc > 1) {
    node_id = strtoul(argv[1], &end, 10);
    start_listen();
    if(argc > 2) {
      unsigned long int msg_id = strtoul(argv[2], &end, 10);
      send_promise_req(msg_id);
    }
  }

  if(pthread_join(t, NULL)) {
    fprintf(stderr, "Error joining thread\n");
    return 2;
  }

  return 0;
}

