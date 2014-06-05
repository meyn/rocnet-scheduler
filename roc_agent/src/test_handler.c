/*
* RoCNet
* Author(s): Feidias, Anselm Meyn
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include "test_handler.h"
#include "config.h"

static const char* RESULT_STRINGS[] = {
  "ROC_ERROR",
  "ROC_NOTSUPPORTED"
};
static unsigned int MAX_RESULT_STRS = sizeof(RESULT_STRINGS)/sizeof(RESULT_STRINGS[0]);
static sTestItemEntry *head = NULL;


/*
*/
void handle_item(char *line)
{
  char* key = NULL;
  char* cmd = NULL;
  int keyLen = 0;
  int cmdLen = 0;
  sTestItemEntry* testItem = NULL;

  key = strtok(line, ",");
  cmd = strtok(NULL, ",");
  //syslog(LOG_DEBUG, ("handle_item(): [%s -> %s]", key, cmd);
  
  if (key != NULL && cmd != NULL) {
    keyLen = strlen(key)+1;
    cmdLen = strlen(cmd)+1;
    testItem = (sTestItemEntry*) calloc(1, sizeof(sTestItemEntry));
    testItem->key = (char*) calloc(sizeof(char), keyLen);
    testItem->cmd = (char*) calloc(sizeof(char), cmdLen);
    
    if (testItem->key != NULL && testItem->cmd != NULL) {
      strcpy(testItem->key, key);
      strcpy(testItem->cmd, cmd);
      testItem->key[keyLen-1] = '\0';
      testItem->cmd[cmdLen-1] = '\0';
      addItem(testItem);
    }
  }
}


/*
*/
size_t get_response_str(unsigned int respCode, char* respStr)
{
  int code = ((respCode >= 0) && (respCode < MAX_RESULT_STRS)) ? respCode: 0;
  size_t len = strlen(RESULT_STRINGS[code]);
  strncpy(respStr, RESULT_STRINGS[code], len);
  respStr[len] = '\0';
  return len;
}


/*
*/
int addItem(sTestItemEntry* testItem)
{
  testItem->next = NULL;
  sTestItemEntry* trav = head;

  if (head == NULL) {
    head = testItem;
  } else {
    while(trav->next != NULL) {
      trav = trav->next;
    }
    trav->next = testItem;
  }

  return 0;
}


/*
*/
void printItemsList()
{
  sTestItemEntry* trav = head;

  while (trav != NULL) {
    printf("%-20s = %s\n", trav->key, trav->cmd);
    trav = trav->next;
  }
}


/*
*/
char* getKeyCommand(const char *keyValue)
{
  sTestItemEntry* trav = head;
  while (trav != NULL) {
    if (strlen(keyValue) == strlen(trav->key)) {
      if (strncmp(trav->key, keyValue, strlen(trav->key)) == 0) {
        return trav->cmd;
      }
    }
    trav = trav->next;
  }
  return NULL;
}


/*
  double forks to avoid zombie process
*/
char* exec_cmd(const char* cmdString, char *resultString)
{
  int pipe_fd[2];
  pid_t child_pid = -1;
  pid_t grandchild_pid = -1;
  int status = 0;
  struct timeval timeout;
  char buf[LINE_MAX];
  int bytesRead = 0;
  fd_set read_fds;
  const char* dest = get_dest();
  const char* zbxConf = get_zbx_conf();

//  syslog(LOG_DEBUG, ("[%d] exec_cmd(): [%s]", getpid(), cmdString);

  if (cmdString == NULL) {
    syslog(LOG_DEBUG, "exec_cmd(): cmd error");
    return strncat(resultString, RESULT_STRINGS[1], strlen(RESULT_STRINGS[1]));
  }

  if (pipe(pipe_fd) < 0) { // create a pipes (0 = read end, 1 = write end)
    syslog(LOG_NOTICE, "exec_cmd(): pipe error: %s", strerror(errno));
    return strncat(resultString, RESULT_STRINGS[0], strlen(RESULT_STRINGS[0]));
  }

  child_pid = fork();
  if (child_pid < 0) {
    syslog(LOG_NOTICE, "exec_cmd(): fork error: %s", strerror(errno));
    return strncat(resultString, RESULT_STRINGS[0], strlen(RESULT_STRINGS[0]));
  }
  else if (child_pid == 0) {
    // child - which will exit immediately after forking a grandchild
    close(pipe_fd[0]);   // child doesn't need the read end of the pipe
    if ((grandchild_pid = fork()) < 0) {
      syslog(LOG_NOTICE, "exec_cmd(): child fork error: %s\n", strerror(errno));
      exit(-1);
    }
    else if (grandchild_pid == 0) {
      // grandchild - which actually executes the command
      close(1);        // close grandchild stdout
      dup(pipe_fd[1]); // and make the stdout the same as the pipe_fd[1]
      execl(cmdString, cmdString, dest, zbxConf, (char*)0);
      exit(0);
    }
    else {
      // child
      //syslog(LOG_DEBUG, ("grandchild_pid[%d]", grandchild_pid);
      sprintf(buf, "%d", grandchild_pid);
      write(pipe_fd[1], buf, strlen(buf));
      exit(0);
    }
  }
  else {
    // parent
    waitpid(child_pid, &status, 0);

    // get grandchild pid - the first string to be written to the pipe
    memset(buf, '\0', sizeof(buf));
    read(pipe_fd[0], buf, 20); // assume max length of pid string is 20
    sscanf(buf, "%d", &grandchild_pid);
    syslog(LOG_DEBUG, "exec_cmd(): [%d->%d->%d: %s %s]",
            getpid(), child_pid, grandchild_pid,
            cmdString, dest);

    timeout.tv_sec  = get_cmd_timeout();
    timeout.tv_usec = 0;
    FD_ZERO(&read_fds);
    FD_SET(pipe_fd[0], &read_fds);
    switch (select(pipe_fd[0]+1, &read_fds, NULL, NULL, &timeout)) {
      case -1:
        syslog(LOG_NOTICE, "exec_cmd(): select() error: %s", strerror(errno));
        return strncat(resultString, RESULT_STRINGS[0], strlen(RESULT_STRINGS[0]));
      case 0:
        syslog(LOG_NOTICE, "exec_cmd(): timeout [%s]", cmdString);
        close(pipe_fd[0]);
        //send a signal the script can trap and cleanup
        kill(grandchild_pid, SIGTERM);
        return strncat(resultString, RESULT_STRINGS[0], strlen(RESULT_STRINGS[0]));
      default:
        memset(buf, '\0', sizeof(buf));
        bytesRead = read(pipe_fd[0], buf, sizeof(buf));
        close(pipe_fd[0]);
        syslog(LOG_DEBUG, "exec_cmd(): read [%d] [%s]", bytesRead, buf);
        return strcat(resultString, buf);
    }
  }

  return NULL;
}
