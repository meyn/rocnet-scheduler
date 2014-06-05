/*
* RoCNet
* Author: Anselm Meyn
*/

#include <iostream>
#include <algorithm>
#include <climits>
#include <libgen.h>
#include <syslog.h>
#include <unistd.h>
#include "scheduler.hh"

using namespace std;

static const char* PROG_ID = "roc_server";


std::string getCmdOption(char ** begin, char ** end, const std::string & option)
{
  char ** itr = std::find(begin, end, option);
  if (itr != end && ++itr != end) {
    return *itr;
  }
  return std::string();
}


bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
  return (std::find(begin, end, option) != end);
}


int main(int argc, char *argv[])
{
  // do all the preliminary setup stuff
  bool dFlag = false;
  cScheduler scheduler;
  std::string dbPath;


  if(cmdOptionExists(argv, argv+argc, "-h")) {
  	cout << "Usage: " << basename(argv[0]) << " -d sqlite3_database_file [-h]" << endl;
  	return 0;
  }

  if (cmdOptionExists(argv, argv+argc, "-D")) {
    dFlag = true;
  }

  dbPath = getCmdOption(argv, argv+argc, "-d");
  if (dbPath.empty()) {
    cout << "no database file specified" << std::endl;
    return -1;
  }
  else {
  	cDB::inst().setDbFilePath(dbPath.c_str());
  }

  if (scheduler.readFromDb()) {
    // setup logging now
    setlogmask(LOG_UPTO(LOG_DEBUG));
    openlog(PROG_ID, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_NOTICE, "%s starting", PROG_ID);

    if (dFlag) {
      daemon(0, 0);
    }
    else {
      cout << scheduler << endl;
    }

    scheduler.startTestLoop();
  }

  return 0;
}
