/*
* RoCNet
* Author: Anselm Meyn
*/

#include <iostream>
#include <assert.h>
#include <iomanip>
#include <syslog.h>
#include "scheduler.hh"

using namespace std;

std::ostream& operator<< (std::ostream& out, cScheduler& scheduler)
{
  map_hosts::iterator host_map_iterator;
  map_tests::iterator test_map_iterator;

  out << "test_map (" << scheduler.test_map.size() << ")\n========" << std::endl; 
  for (test_map_iterator = scheduler.test_map.begin();
       test_map_iterator != scheduler.test_map.end();
       ++test_map_iterator) {
    out << test_map_iterator->second << std::endl;
  }

  out << "host_map (" << scheduler.host_map.size() << ")\n========" << std::endl;
  for (host_map_iterator = scheduler.host_map.begin();
       host_map_iterator != scheduler.host_map.end();
       ++host_map_iterator) {
    out << host_map_iterator->second << std::endl;
  }

  out << "\nscheduler_queue (" << scheduler.scheduler_queue.size() << ")\n---------------" << std::endl;

  return out;
}


cScheduler::cScheduler()
{
//  std::cout << "cScheduler::ctor(): " << std::endl;
}


bool cScheduler::readFromDb()
{
  using namespace std::placeholders;
  if (cDB::inst().open()) {
    cDB::inst().stepTestHostMap(std::bind(&cScheduler::addHostEntry, this, _1, _2),
                                std::bind(&cScheduler::addTest2Host, this, _1, _2));
    return true;
  }
  return false;
}


cScheduler::~cScheduler()
{
  cDB::inst().close();
}


void cScheduler::addHostEntry(const sHostEntry& hostEntry, const sHostIfEntry& hostIfEntry)
{
  if (host_map.insert(std::make_pair(hostEntry.id, cHost(hostEntry))).second == true) {
    host_map[hostEntry.id].calcNextTestTime();
    scheduler_queue.push(&(host_map[hostEntry.id]));
  }
  host_map[hostEntry.id].addIf(hostIfEntry.id, hostIfEntry);
  assert(scheduler_queue.size() <= host_map.size());
}


void cScheduler::addTest2Host(UINT hostId, const sTestEntry& testEntry)
{
  test_map.insert(std::make_pair(testEntry.id, cTest(testEntry)));
  host_map[hostId].addTest(testEntry.id, &test_map[testEntry.id]);
}


cHost* cScheduler::getNextHost()
{
  return NULL;
}


void cScheduler::startTestLoop()
{
  struct timeval currTime;
  struct timespec sleepTime{ 0, 0 };
  time_t hostNextRunTime;
  cHost *host;

  while(1) {

    while (!scheduler_queue.empty()) {
      gettimeofday(&currTime, NULL);

      host = scheduler_queue.top();
      hostNextRunTime = host->getNextRunTime();

      syslog(LOG_NOTICE, "testing %s in %lds", host->getHostName().c_str(), (hostNextRunTime - currTime.tv_sec));

      if (hostNextRunTime >= currTime.tv_sec) {
        sleepTime.tv_sec = hostNextRunTime - currTime.tv_sec;
        nanosleep(&sleepTime, NULL);
      }

      scheduler_queue.pop();
      if (host->startTest()) {
        cDB::inst().updateRowHostRunInfo(host->getHostId(), host->getLastRun());
      }
      host->calcNextTestTime();
      scheduler_queue.push(host);
    }

    // Nothing to do so check every 10 seconds
    sleepTime.tv_sec = 10;
    nanosleep(&sleepTime, NULL);

    syslog(LOG_DEBUG, "waking up to check");
  }
}


void cScheduler::alarmHandler(int signum)
{
  std::cout << "cScheduler::alarmHandler()" << std::endl;
}
