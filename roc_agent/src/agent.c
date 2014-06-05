/*
* RoCNet
* Author: Anselm Meyn
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "agent.h"
#include "test_handler.h"
#include "config.h"

static struct sockaddr_in local_end;


void daemonize()
{
  pid_t pid, sid;

  // check that we are not already a daemon
  if (getppid() == 1) return;

  // fork off a new process
  pid = fork();

  // fork failed
  if (pid < 0) {
    fprintf(stderr, "fork error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  // fork succeeded so exit the parent process
  if (pid > 0) {
    printf("exiting %d\n", getpid());
    exit(EXIT_SUCCESS);
  }
 
  // In child process at this point.
  // 1. change the file mode creation mask
  umask(0);

  // 3. create a new sid for the child process
  sid = setsid();
  if (sid < 0) {
    fprintf(stderr, "setsid error: %s", strerror(errno));
    syslog(LOG_ERR, "exiting with setsid error");
    exit(EXIT_FAILURE);
  }

  // 4. change working directory
  if (chdir("/") < 0) {
    fprintf(stderr, "chdir error: %s", strerror(errno));
    syslog(LOG_ERR, "exiting with chdir error");
    exit(EXIT_FAILURE);
  }

  // 5. close the standard file descriptors now
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}


void start_agent()
{
  int server_fd, client_fd;

  // now start waiting for connections
  while (1) {
    server_fd = start_listener();
    if (server_fd > 0) {
      client_fd = await_connection(server_fd);
      if (client_fd > 0) {
        handle_input(client_fd);
      }
    } else {
      syslog(LOG_ERR, "error starting agent");
      exit(EXIT_FAILURE);
    }
  }
  exit(EXIT_SUCCESS);
}


int setup_server()
{
  const char *ifName = get_interface_name();
  const unsigned short port = get_port();

  if (ifName == NULL || port == 0) {
    syslog(LOG_ERR, "invalid server parameters\n");
    return -1;
  }

  memset((char *)&local_end, 0, sizeof(local_end));
  local_end.sin_family = AF_INET; // AddressFamily = Internet address
  //local_end.sin_addr.s_addr = get_interface_ip(ifName); //INADDR_ANY;//
  if (get_interface_ip(ifName, &local_end.sin_addr) != 0) { //INADDR_ANY;//
    return -1;
  }
  local_end.sin_port = htons(port);

  return 0;
}


int start_listener()
{
  int srv_sockd = -1;
  int optval = 1;

  srv_sockd = socket(AF_INET, SOCK_STREAM, 0);
  if(srv_sockd < 0) {
    syslog(LOG_ERR, "start_listener(): socket error: %s", strerror(errno));
    return -1;
  }

/*
  if (setsockopt(srv_sockd, SOL_SOCKET, SO_BINDTODEVICE, ifName, strlen(ifName)) < 0) {
    syslog(LOG_ERR,"Failed setting socket option");
    return -1;
  }
*/

  if (setsockopt(srv_sockd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
    syslog(LOG_ERR, "start_listener(): setsockopt error: %s", strerror(errno));
    return -1;
  }

  // bind the address to the socket file descriptor
  if (bind(srv_sockd, (struct sockaddr *)&local_end, sizeof(local_end)) < 0) {
    syslog(LOG_ERR, "start_listener(): bind error: %s", strerror(errno));
    close(srv_sockd);
    return -1;
  }

  // Now start listening
  if (listen(srv_sockd, 1) < 0) {
    syslog(LOG_ERR, "start_listener(): listen error: %s", strerror(errno));
    close(srv_sockd);
    return -1;
  }

  syslog(LOG_DEBUG,  "start_listener(): listen [%d],[%s:%hu]", srv_sockd, inet_ntoa(local_end.sin_addr), ntohs(local_end.sin_port));

  return srv_sockd;
}


int await_connection(int server_fd)
{
  struct sockaddr_in remote_endp;
  socklen_t size = sizeof(remote_endp);
  char *remote_addr;
  const char *scheduler_addr = get_scheduler();
  int remote_fd;

  remote_fd = accept(server_fd, (struct sockaddr *)&remote_endp, &size);
  // close server as soon as we accept a connection
  close(server_fd);

  if (remote_fd < 0) {
    syslog(LOG_WARNING, "await_connection(): accept error: %s", strerror(errno));
  } else {
    remote_addr = inet_ntoa(remote_endp.sin_addr);
    if (strncmp(scheduler_addr, remote_addr, strlen(scheduler_addr)) != 0) {
      close(remote_fd);
      syslog(LOG_DEBUG,  "await_connection(): rejected [%d] [%s:%hu]", remote_fd, remote_addr, ntohs(remote_endp.sin_port));
      remote_fd = -1;
    } 
    else {
      syslog(LOG_DEBUG,  "await_connection(): accepted [%d], [%s:%hu]", remote_fd, remote_addr, ntohs(remote_endp.sin_port));      
    }
  }

  return remote_fd;
}


char is_valid_char(char ch)
{
  if (isalnum(ch) ||
      ch == '.' || ch == '_' || ch == ',' || ch == '-' || ch == '+' ||
      ch == '[' || ch == ']' || ch == '{' || ch == '}') {
    return 1;
  }
  return 0;
}


int handle_input(int fd)
{
  struct timeval recv_timeout;
  char buff[LINE_MAX];
  char ch;
  int bytesRead = 0;
  int err_count = 0;
  int i;
  char *cmd = NULL;

  memset(buff, '\0', sizeof(buff));
  recv_timeout.tv_sec = RD_TIMEOUT;
  recv_timeout.tv_usec = 0;

  if (fd < 0) {
    return -1;
  }

  if (setsockopt (fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&recv_timeout, sizeof(recv_timeout)) < 0) {
    syslog(LOG_WARNING, "handle_input(): [%d] setsockopt error: %s", fd, strerror(errno));
    return -1;
  }

  while ((bytesRead = read(fd, buff, sizeof(buff)-1)) > 0) {
    bytesRead = (bytesRead > LINE_MAX) ? LINE_MAX-1 : bytesRead;
    for (i=0; i<bytesRead; ++i) {
      ch = buff[i];
      if (!is_valid_char(ch)) {
        buff[i] = '\0';
        break;
      }
    }
    
    cmd = getKeyCommand(buff);
    syslog(LOG_DEBUG,  "handle_input(): [%d] read [%s]", fd, buff);
    memset(buff, '\0', bytesRead);
    if (cmd == NULL) {
      if (++err_count >= MAX_ERRORS) {
        syslog(LOG_DEBUG, "handle_input(): [%d] too many errors", fd);
        break;
      }
      get_response_str(1, buff); // 1 refers to UNSUPPORTED
    }
    else {
      exec_cmd(cmd, buff);
    }

    // send the result back
    if (write(fd, buff, strlen(buff)) < 0) {
      syslog(LOG_WARNING, "handle_input(): [%d] write error [%s]", fd, strerror(errno));
    }
    syslog(LOG_DEBUG,  "handle_input(): [%d] write [%s]", fd, buff);
    fflush(NULL);
    memset(buff, '\0', sizeof(buff));
  }

  close(fd);
  syslog(LOG_DEBUG,  "handle_input(): [%d] close", fd);

  return 0;
}


int get_interface_ip(const char* ifName, struct in_addr* ifAddr)
{
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  int retval = 0;
  struct ifreq ifr;

  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, ifName, IFNAMSIZ-1);
  if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) {
    syslog(LOG_WARNING, "ioctl error: %s\n", strerror(errno));
    fprintf(stderr, "ioctl error: %s\n", strerror(errno));
    retval = -1;
  }
  else {
    *ifAddr = ((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr;
  }
  close(sockfd);
  return retval;
}
