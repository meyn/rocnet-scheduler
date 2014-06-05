#ifndef __CONFIG_H__
#define __CONFIG_H__
/*
* RoCNet
* Author: Anselm Meyn
*/

#include <time.h>
#include <limits.h>

typedef enum CONFIG_OPTS {
  PORT = 0,
  INTERFACE,
  ZBX_CONF,
  TESTSERVER,
  TESTITEM,
  TIMEOUT,
  SCHEDULER,
  MAX
} CONFIG_OPTS;

int parse_file(const char* filePath);
CONFIG_OPTS getOptType(const char* line);

void set_port(const char* line);
void set_interface(const char* line);
void set_cmd_timeout(const char* line);
void set_dest(const char* line);
void set_zbx_conf(const char* line);
void set_scheduler(const char* line);

const unsigned short get_port();
const char* get_interface_name();
const char* get_dest();
const time_t get_cmd_timeout();
const char* get_zbx_conf();
const char* get_scheduler();

void print_config();

#endif /*__CONFIG_H__*/
