#include <stdio.h>
#include <stdlib.h>

#include "citcomm.h"

unsigned long int node_id = -1;
unsigned long int promise = -1;
unsigned long int commit = -1;

unsigned long int convert_ids(struct Payload payload) {
  char payload_num_str[10];
  sprintf(payload_num_str, "%lu%lu", 
          payload.msg_id,
          payload.node_id);

  char* end;
  return strtoul(payload_num_str, &end, 10);
}

void handle_payload(struct Payload payload) {

  // Don't handle my own messages - JBG
  if(payload.node_id == node_id) return;

  printf("Payload: %d %lu%lu\n",
         payload.cmd,
         payload.msg_id,
         payload.node_id);

  unsigned long int n = convert_ids(payload);

  if(n > promise) {
    promise = n;
    printf("Promise: %lu\n", promise);
  }

}

int main(int argc, char **argv) {

  char* end;
  if(argc < 1 || argc > 3) {
    printf("USAGE: citizen node-id msg-id\n");
    return 1;
  } else if(argc > 2) {
    struct Payload payload;
    payload.cmd = PROMISE;
    payload.node_id = strtoul(argv[1], &end, 10);
    payload.msg_id = strtoul(argv[2], &end, 10);

    //handle_payload(payload); 
    client_send(payload);
  } else {
    node_id = strtoul(argv[1], &end, 10);
    printf("I'm NODE ID: %lu\n", node_id);
    void (*handler)(struct Payload);
    handler = &handle_payload;
    server_listen(handler);
  }

//  pthread_t t;
//  if(pthread_create(&t, NULL, server_listen, NULL)) {
//    fprintf(stderr, "Error creating thread\n");
//    return 1;
//  }


  return 0;
}

