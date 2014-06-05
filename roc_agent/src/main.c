/*
* RoCNet
* Author: Anselm Meyn
*/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include "version.h"
#include "config.h"
#include "agent.h"
#include "test_handler.h"

static const char* FILE_PATH = "etc/roc_agent_lo.conf";
static const char* PROG_ID = "roc_agent";

int main (int argc, char *argv[])
{
  int index;
  int c;
  int dflag = 0;
  char *cvalue = NULL;

  while ((c = getopt(argc, argv, "c:Dhv")) != -1) {
    switch (c) {
    case 'c':
      cvalue = optarg;
      break;
    case 'D':
      dflag = 1;
      break;
    case 'h':
      printf("Usage: %s [-c config_file] [-v] [-h]\n", argv[0]);
      return 0;
    case 'v':
      printf("%s v%s (revision %s) (%s)\n", APPLICATION_NAME, ROCNET_VERSION, ROCNET_REVISION, ROCNET_REVDATE);
      return 0;
    case '?':
      if (optopt == 'c') {
        fprintf(stderr, "-%c missing argument.\n", optopt);
      }
      else if (isprint (optopt)) {
        fprintf (stderr, "Unknown option `-%c'.\n", optopt);
      }
      else {
        fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
      }
      return 1;
    default:
      abort();
    }
  }

  for (index = optind; index < argc; index++) {
    printf("Non-option argument %s\n", argv[index]);
  }

  // setup logging now
  setlogmask(LOG_UPTO(LOG_DEBUG));
  openlog(PROG_ID, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
  syslog(LOG_NOTICE, "%s starting", PROG_ID);

  if (cvalue){
    parse_file(cvalue);
  }
  else {
    parse_file(FILE_PATH);
  }

  if (setup_server() < 0) {
    syslog(LOG_ERR, "%s exiting [setup_server() failed]", PROG_ID);
    exit(EXIT_FAILURE);
  }

  // should we run as a daemon
  if (dflag) {
    if (daemon(0,0) < 0) {
      syslog(LOG_ERR, "error creating daemon: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
  else {
    print_config();
    printf("-------\n");
    printItemsList();
    printf("\t\t-------\n");
  }

  start_agent();

  return 0;
}
