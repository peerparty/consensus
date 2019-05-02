#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "citcomm.h"

unsigned long int node_id = 0;
unsigned long int promise = 0;
unsigned long int commit = 0;

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

void send_promise(unsigned long int dest_id,
                  unsigned long int msg_id) {
  struct Payload payload;
  payload.cmd = PROMISE;
  payload.node_id = node_id;
  payload.msg_id = msg_id;
  payload.dest_id = dest_id;
  client_send(payload);
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

  if(payload.cmd == REQUEST && payload.msg_id > promise) {
    promise = payload.msg_id;
    printf("Promise: %lu\n", promise);
    send_promise(payload.node_id, payload.msg_id);
  } else if(payload.cmd == PROMISE &&
            payload.dest_id == node_id) {
    printf("Promise from: %lu\n", payload.node_id);
  } else {
    printf("Ignored: %d %d\n",
           payload.cmd == REQUEST,
           payload.msg_id > promise);
  }
}

void start_listen() {
  void (*handler)(struct Payload);
  handler = &handle_payload;

  if(pthread_create(&t, NULL, server_listen, handler)) {
    fprintf(stderr, "Error creating thread\n");
    return 1;
  }

}

int main(int argc, char **argv) {

  char* end;

  if(argc < 1 || argc > 3) {
    printf("USAGE: citizen node-id msg-id\n");
    return 1;
  } else if(argc > 2) {
    node_id = strtoul(argv[1], &end, 10);
    struct Payload payload;
    payload.cmd = REQUEST;
    payload.node_id = strtoul(argv[1], &end, 10);
    payload.msg_id = strtoul(argv[2], &end, 10);
    start_listen();
    client_send(payload);
  } else {
    node_id = strtoul(argv[1], &end, 10);
    printf("I'm NODE ID: %lu\n", node_id);
    start_listen();
  }

  if(pthread_join(t, NULL)) {
    fprintf(stderr, "Error joining thread\n");
    return 2;
  }

  return 0;
}

