/*
* RoCNet
* Author: Anselm Meyn
*/

#include <iostream>
#include <iomanip>
#include <cstring>
#include <assert.h>
#include <limits.h>
#include <syslog.h>
#include <errno.h>
#include <stdio.h>
#include "hosts.hh"

using namespace std;

std::ostream& operator<< (std::ostream& out, cHost& host)
{
  map_hostIfs::iterator if_iterator;
  map_testPtrs::iterator test_iterator;

  out << "[" << std::setw(2) << host.sHost.id << ','
      << std::setw(5) << host.sHost.name << ','
      << std::setw(5) << host.sHost.rptInterval << ','
      << std::setw(10) << host.sHost.lastRun
      << "], [" << std::setw(10) << host.nextRun << ", " << host.getExpectedDuration()
      << "], (" << &host << ')' << std::endl;

  for (if_iterator = host.map_ifs.begin(); if_iterator != host.map_ifs.end(); ++if_iterator) {
    out << "  if: " << if_iterator->second << std::endl;
  }

  for (test_iterator = host.map_tests.begin(); test_iterator != host.map_tests.end(); ++test_iterator) {
    out << "  test: " << *(test_iterator->second) << std::endl;
  }
  
  return out;
}


cHost::cHost()
{
  sHost.id = 0;
  sHost.name.clear();
  sHost.rptInterval = 0;
  sHost.lastRun = 0;

  nextRun = 0;

  map_ifs.clear();
  map_tests.clear();
  expectedDuration = 0;
}


cHost::cHost(const sHostEntry& host)
{
  sHost.id = host.id;
  sHost.name = host.name;
  sHost.rptInterval = host.rptInterval;
  sHost.lastRun = host.lastRun;

  nextRun = 0;
  expectedDuration = 0;
}


/*cHost::~cHost() {
  std::cout << "cHost::dtor (" << sHost.id << ")=" << this << std::endl;
}*/


cHost& cHost::operator=(const cHost& rhs)
{
  sHost.id = rhs.sHost.id;
  sHost.name = rhs.sHost.name;
  sHost.rptInterval = rhs.sHost.rptInterval;
  sHost.lastRun = rhs.sHost.lastRun;

  nextRun = rhs.nextRun;
  expectedDuration = rhs.expectedDuration;

  return *this;
}


int cHost::operator==(const cHost &rhs) const
{
  if (this->sHost.id == rhs.sHost.id) {
    return 1;
  }
  return 0;
}


int cHost::operator<(const cHost &rhs) const
{
  return (this->nextRun < rhs.nextRun) ? 1 : 0;
}


UINT cHost::calcNextTestTime()
{
  struct timeval currTime;

  gettimeofday(&currTime, NULL);
	nextRun = sHost.lastRun;
	while (nextRun <= currTime.tv_sec) {
		nextRun += sHost.rptInterval;
	}
  return nextRun;
}


void cHost::addIf(UINT ifId, const sHostIfEntry& hostIf)
{
  if (map_ifs.insert(std::make_pair(ifId, hostIf)).second == true) {
    expectedDuration += expectedDuration;
  }
}


void cHost::addTest(UINT testId, cTest* item)
{
  if ((this->map_tests.insert(std::make_pair(testId, item))).second == true) {
    expectedDuration += item->getExpectedDuration();
  }
  assert(sHost.rptInterval > expectedDuration);
}


bool cHost::startTest()
{
  map_testPtrs::iterator test_iterator;
  map_hostIfs::iterator if_iterator;
  struct timeval currTime;
  char buff[LINE_MAX];
  int connect_fails = 0;

  // update the time we start the test attempt
  gettimeofday(&currTime, NULL);
  sHost.lastRun = currTime.tv_sec;

  for (if_iterator = map_ifs.begin(); if_iterator != map_ifs.end(); ++if_iterator) {
    if (if_iterator->second.connectToAgent() == true) {
      syslog(LOG_DEBUG, "starting tests for %s", sHost.name.c_str());

      for (test_iterator = map_tests.begin(); test_iterator != map_tests.end(); ++test_iterator) {
        syslog(LOG_DEBUG, "test id %d: %p=%s", test_iterator->first, test_iterator->second, test_iterator->second->getKey().c_str());
        memset(buff, '\0', sizeof(buff));
        if (if_iterator->second.sendTestItem(test_iterator->second->getKey()) == true) {
          if_iterator->second.getTestResult(buff, LINE_MAX, test_iterator->second->getExpectedDuration());
          syslog(LOG_DEBUG, "rx: %s [%s] ", cTest::isValidResult(buff) ? "valid" : "invalid", buff);
        }
        else {
          syslog(LOG_WARNING, "failed sending %s", test_iterator->second->getKey().c_str());
        }
      }
      // close connectio now
      if_iterator->second.disconnect();
    }
    else {
      ++connect_fails;
    }
  }

  if (connect_fails) {
    return false;
  }

  return true;
}
