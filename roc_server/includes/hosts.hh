#ifndef __HOSTS_H__
#define __HOSTS_H__
/*
* RoCNet
* Author: Anselm Meyn
*/

#include <map>
#include "types.hh"
#include "hostIf.hh"
#include "tests.hh"

typedef struct sHostEntry_t {
  UINT id;
  std::string name;
  UINT rptInterval;
  UINT lastRun;
} sHostEntry;

typedef std::map<UINT, cHostIf> map_hostIfs;
typedef std::map<UINT, cTest*> map_testPtrs;

class cHost {
private:
  sHostEntry sHost;
  map_hostIfs  map_ifs;
  map_testPtrs map_tests;

  time_t nextRun;
  UINT expectedDuration;

public:
  cHost();
  cHost(const sHostEntry& host);
  //virtual ~cHost();

  cHost& operator=(const cHost& rhs);
  int operator==(const cHost &rhs) const;
  int operator<(const cHost &rhs) const;
  friend std::ostream& operator<< (std::ostream& out, cHost& host);

  UINT calcNextTestTime();
  void addIf(UINT ifId, const sHostIfEntry& hostIf);
  void addTest(UINT testId, cTest* testItem);
  bool startTest();
  UINT getHostId() const { return sHost.id; }
  std::string getHostName() const { return sHost.name; }
  time_t getLastRun() const { return sHost.lastRun; }
  time_t getNextRunTime() const { return nextRun; };
  UINT getExpectedDuration() const { return expectedDuration; };
};

#endif /* __HOSTS_H__ */
