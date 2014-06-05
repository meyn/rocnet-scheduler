#ifndef __TESTS_H__
#define __TESTS_H__
/*
* RoCNet
* Author: Anselm Meyn
*/

#include <string>
#include "types.hh"

typedef struct sTestEntry_t {
  UINT id;
  std::string key;
  UINT duration;
  std::string srvSetup;
} sTestEntry;

class cTest {
private:
  sTestEntry testItem;

public:
  static const char* resultErrStr[];

  cTest();
  cTest(const sTestEntry& testItem);

  cTest& operator=(const cTest& rhs);
  int operator==(const cTest &rhs) const;
  int operator<(const cTest &rhs) const;
  friend std::ostream& operator<< (std::ostream& out, cTest& test);

  std::string getKey() const { return testItem.key; }
  UINT getExpectedDuration() const { return testItem.duration; }
  std::string getSrvSetupCmd() const { return testItem.srvSetup; }

  static bool isValidResult(const char*);
};

#endif /* __TESTS_H__ */
