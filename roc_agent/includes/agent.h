#ifndef __AGENT_H__
#define __AGENT_H__
/*
* RoCNet
* Author: Anselm Meyn
*/

#include <netinet/in.h>

// socket read timeout in seconds
#define RD_TIMEOUT 10
#define MAX_ERRORS 3

void daemonize();
void start_agent();
int  setup_server();
int  start_listener();
int  await_connection(int fd);
int  get_interface_ip(const char*, struct in_addr*);
char is_valid_char(char ch);
int  handle_input(int fd);

#endif /* __AGENT_H__ */
