#ifndef __HOSTIF_H__
#define __HOSTIF_H__
/*
* RoCNet
* Author: Anselm Meyn
*/

#include <arpa/inet.h>
#include <sys/time.h>
#include "tests.hh"

typedef struct sHostIfEntry_t {
  UINT id;
  std::string ipv4Addr;
  std::string dns;
  UINT port;
} sHostIfEntry;

class cHostIf {
private:
  sHostIfEntry sHostIf;
  struct sockaddr_in clientListener;
  int sockfd;

public:
  cHostIf();
  cHostIf(const sHostIfEntry& hostIf);
  cHostIf& operator= (const cHostIf& rhs);
  int operator== (const cHostIf& rhs) const;
  friend std::ostream& operator<< (std::ostream& out, cHostIf& hostIf);

  void  addTest(UINT testId, cTest* testItem);
  bool  connectToAgent();
  bool  sendTestItem(std::string key);
  bool  is_valid_char(char ch);
  char* getTestResult(char* resultBuff, size_t buffLen, time_t duration);
  void  disconnect();
};

#endif /* __HOSTIF_H__ */
