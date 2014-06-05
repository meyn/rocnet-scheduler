/*
* RoCNet
* Author: Anselm Meyn
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "agent.h"
#include "test_handler.h"

#ifndef LINE_MAX
  #define LINE_MAX 1024
#endif
#define DEFAULT_PORT 10057
#define CMD_TIMEOUT 15


static const char *LINE_IDENTIFIERS[] = {
  "ListenPort=",
  "Interface=",
  "ZabbixConfig=",
  "Destination=",
  "TestItem=",
  "Timeout=",
  "Scheduler="
};
static unsigned short port    = DEFAULT_PORT;
static char ifName[255]       = "lo";
static char destination[64]   = "127.0.0.1";
static char scheduler[64]     = "127.0.0.1";
static char zbxConf[LINE_MAX] = "/opt/zabbix/etc/zabbix_agentd.conf";
static time_t cmdTimeout      = 20; // netperf takes 63s to fail


int parse_file(const char *filePath)
{
  size_t line_len = 0;
  char line[LINE_MAX];
  FILE* fp = NULL;

  if (!filePath) {
    return -1;
  }

  fp = fopen(filePath, "r");

  if (fp != NULL) {
    memset(line, '\0', sizeof(line));

    while(fgets(line, sizeof(line), fp) != NULL) {
      line_len = strlen(line);
      if (line_len && (('\n' == line[line_len-1]) || ('\r' == line[line_len-1]))) {
        line[line_len-1] = '\0';
        --line_len;
      }

      switch(getOptType(line)) {
        case PORT:
          set_port(&line[strlen(LINE_IDENTIFIERS[PORT])]);
          break;
        case INTERFACE:
          set_interface(&line[strlen(LINE_IDENTIFIERS[INTERFACE])]);
          break;
        case ZBX_CONF:
          set_zbx_conf(&line[strlen(LINE_IDENTIFIERS[ZBX_CONF])]);
          break;
      	case TESTSERVER:
          set_dest(&line[strlen(LINE_IDENTIFIERS[TESTSERVER])]);
      	  break;
      	case TESTITEM:
      	  handle_item(&line[strlen(LINE_IDENTIFIERS[TESTITEM])]);
      	  break;
        case TIMEOUT:
          set_cmd_timeout(&line[strlen(LINE_IDENTIFIERS[TIMEOUT])]);
          break;
        case SCHEDULER:
          set_scheduler(&line[strlen(LINE_IDENTIFIERS[SCHEDULER])]);
          break;
      	default:
      	  break;
      }
    }
    fclose(fp);
  } else {
    fprintf(stderr, "error opening config file (%s) for reading\n", filePath);
    return -1;
  }

  return 0;
}


CONFIG_OPTS getOptType(const char *line)
{
  int i;

  for (i=0; i<MAX; ++i) {
    if (strncmp(LINE_IDENTIFIERS[i], line, strlen(LINE_IDENTIFIERS[i])-1) == 0) {
      return i;
    }
  }

  return MAX;
}


void set_port(const char* line)
{
  if (line) {
    sscanf(line, "%hu", &port);
  }
  if (port <= 1024) {
    port = DEFAULT_PORT;
  }
}

const unsigned short get_port()
{
  return port;
}


void set_interface(const char* line)
{
  if (line) {
    strncpy(ifName, line, sizeof(ifName));
  }
}

const char* get_interface_name()
{
  return ifName;
}


void set_dest(const char *line)
{
  if (line) {
    strncpy(destination, line, sizeof(destination));
  }
}

const char* get_dest()
{
  return destination;
}


void set_cmd_timeout(const char* line)
{
  if (line) {
    sscanf(line, "%ld", &cmdTimeout);
  }
}

const time_t get_cmd_timeout()
{
  return cmdTimeout;
}


void set_zbx_conf(const char *line)
{
  if (line) {
    strncpy(zbxConf, line, sizeof(zbxConf));
  }
}

const char* get_zbx_conf()
{
  return zbxConf;
}


void set_scheduler(const char* line)
{
  if (line) {
    strncpy(scheduler, line, sizeof(scheduler));
  }
}

const char* get_scheduler()
{
  return scheduler;
}

void print_config()
{
  printf("agent config\n"
          "\tinterface: %s\n"
          "\tport: %hu\n"
          "\tscheduler: %s\n"
          "\tZabbix config: %s\n"
          "\tcommand timeout: %ld\n"
          "\tcommand destination: %s\n",
          ifName,
          port,
          scheduler,
          zbxConf,
          cmdTimeout,
          destination);
}