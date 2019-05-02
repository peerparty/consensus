#ifndef CITCOMM_H 
#define CITCOMM_H 

enum Cmd {
  ONLINE,
  PROMISE,
  COMMIT
};

struct Payload {
  enum Cmd cmd;
  unsigned long int node_id;
  unsigned long int msg_id;
};

int client_send(struct Payload payload);
int server_listen(void(*handle_payload)(struct Payload));

#endif //CITCOMM_H

