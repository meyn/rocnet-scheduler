#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__
/*
* RoCNet
* Author: Anselm Meyn
*/

#include <queue>
#include "types.hh"
#include "hosts.hh"
#include "tests.hh"
#include "db.hh"

struct OrderByTime {
	bool operator() (const cHost* lhs, const cHost* rhs) {
		return !(*lhs < *rhs);
	}
};

typedef std::map<UINT, cHost> map_hosts;
typedef std::map<UINT, cTest> map_tests;
typedef std::priority_queue<cHost*, std::vector<cHost*>, OrderByTime> queue_hosts;


class cScheduler {
private:
  map_hosts host_map;
  map_tests test_map;
  queue_hosts scheduler_queue;

public:
  cScheduler();
  virtual ~cScheduler();
  friend std::ostream& operator<< (std::ostream& out, cScheduler& scheduler);

  bool readFromDb();
  void addHostEntry(const sHostEntry& hostEntry, const sHostIfEntry& hostIfEntry);
  void addTest2Host(UINT hostId, const sTestEntry& testEntry);
  void startTestLoop();
  static void alarmHandler(int signum);

  cHost* getNextHost();
};

#endif /* __SCHEDULER_H__ */
