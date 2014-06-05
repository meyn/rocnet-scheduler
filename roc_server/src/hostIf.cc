/*
* RoCNet
* Author: Anselm Meyn
*/

#include <iostream>
#include <iomanip>
#include <cstring>
#include <unistd.h>
#include <syslog.h>
#include "hostIf.hh"

std::ostream& operator<< (std::ostream& out, cHostIf& hostIf)
{
  out << "[" << std::setw(2) << hostIf.sHostIf.id << ','
      << std::setw(15) << hostIf.sHostIf.ipv4Addr << ','
      << std::setw(15) << hostIf.sHostIf.dns << ','
      << std::setw(7) << hostIf.sHostIf.port << "], (" << &hostIf << ")";
  
  return out;
}


cHostIf::cHostIf()
{
  sHostIf.id = 0;
  sHostIf.ipv4Addr.clear();
  sHostIf.dns.clear();
  sHostIf.port = 0;

  memset(&clientListener, '0', sizeof(clientListener));
  clientListener.sin_family = AF_INET;
  clientListener.sin_addr.s_addr = htonl(0); // 0
  clientListener.sin_port = htons(sHostIf.port);
  sockfd = -1;
}


cHostIf::cHostIf(const sHostIfEntry& hostIf)
{
  sHostIf.id = hostIf.id;
  sHostIf.ipv4Addr = hostIf.ipv4Addr;
  sHostIf.dns = hostIf.dns;
  sHostIf.port = hostIf.port;

  memset(&clientListener, '0', sizeof(clientListener));
  clientListener.sin_family = AF_INET;
  if (inet_pton(AF_INET, sHostIf.ipv4Addr.c_str(), &clientListener.sin_addr) <= 0) {
    syslog(LOG_WARNING, "inet_pton error: %s", strerror(errno));
    throw -1;
  }
  clientListener.sin_port = htons(sHostIf.port);
  sockfd = -1;
}


cHostIf& cHostIf::operator=(const cHostIf& rhs)
{
  this->sHostIf.id = rhs.sHostIf.id;
  this->sHostIf.ipv4Addr = rhs.sHostIf.ipv4Addr;
  this->sHostIf.dns = rhs.sHostIf.dns;
  this->sHostIf.port = rhs.sHostIf.port;

  return *this;
}


int cHostIf::operator==(const cHostIf &rhs) const
{
  if ((this->sHostIf.id == rhs.sHostIf.id) ||
      (this->sHostIf.ipv4Addr == rhs.sHostIf.ipv4Addr && this->sHostIf.port == rhs.sHostIf.port) ||
      (this->sHostIf.dns == rhs.sHostIf.dns && this->sHostIf.port == rhs.sHostIf.port)) {
    return 1;
  }
  return 0;
}


bool cHostIf::connectToAgent()
{
  struct timeval timeout;

  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    syslog(LOG_WARNING, "socket() error: %s", strerror(errno));
    return false;
  }

  syslog(LOG_INFO, "[%d] connecting to %s:%hu", sockfd, inet_ntoa(clientListener.sin_addr), ntohs(clientListener.sin_port));

  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
    syslog(LOG_WARNING, "setsockopt() error: %s", strerror(errno));
    close(sockfd);
  }

  if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
    syslog(LOG_WARNING, "setsockopt() failed");
    close(sockfd);
  }

  if (connect(sockfd, (struct sockaddr*)&clientListener, sizeof(clientListener)) < 0) {
    syslog(LOG_WARNING, "[%d] connect() error: %s", sockfd, strerror(errno));
    close(sockfd);
    return false;
  }

  return true;
}


bool cHostIf::sendTestItem(std::string key)
{
  char sendBuf[1024];
  //UINT len = snprintf(sendBuf, sizeof(sendBuf), "%s,%u\n", key.c_str(), duration);
  UINT len = snprintf(sendBuf, sizeof(sendBuf), "%s\n", key.c_str());
  syslog(LOG_DEBUG, "sent key: %s", sendBuf);
  return (write(sockfd, sendBuf, len) == -1 ? false : true);
}


bool cHostIf::is_valid_char(char ch)
{
  if (isalnum(ch) ||
      ch == '.' || ch == '_' || ch == ',' || ch == '-' || ch == '+' ||
      ch == '[' || ch == ']' || ch == '{' || ch == '}') {
    return true;
  }
  return false;
}


char* cHostIf::getTestResult(char *resultBuff, size_t buffLen, time_t duration)
{
  struct timeval timeout;
  fd_set read_fds;
  size_t bytesRead = 0;
  size_t buffStrLen = 0;
  const char* firstNonSpace;
  size_t firstValidCharPos;
  size_t i;
  
  timeout.tv_sec = duration;
  timeout.tv_usec = 0;
  FD_ZERO(&read_fds);
  FD_SET(sockfd, &read_fds);

  switch (select(sockfd+1, &read_fds, NULL, NULL, &timeout)) {
    case -1:
      syslog(LOG_WARNING, "select() error: %s", strerror(errno));
      return strncat(resultBuff, "ROC_READERROR", 14);
    
    case 0:
      syslog(LOG_WARNING, "timed out");
      return strncat(resultBuff, "ROC_TIMEOUT", 14);
    
    default:
      bytesRead = read(sockfd, resultBuff, buffLen);
      firstNonSpace = resultBuff;
      firstValidCharPos = 0;
      resultBuff[buffLen-1] = '\0'; // terminate the last char anyway
      buffStrLen = (bytesRead < buffStrLen) ? bytesRead : buffStrLen;

      // remove leading whitespace
      while (*firstNonSpace != '\0' && isspace(*firstNonSpace)) {
        ++firstNonSpace;
      }
      buffStrLen = strlen(firstNonSpace)+1;
      memmove(resultBuff, firstNonSpace, buffStrLen);

      // now end it at the first unsupported char
      for (i=0; i<buffStrLen; ++i) {
        if (!is_valid_char(resultBuff[i])) {
          if (!firstValidCharPos)
          resultBuff[i] = '\0';
          break;
        }
      }
      syslog(LOG_DEBUG, "received: %s", resultBuff);
      return resultBuff;
  }
}


void cHostIf::disconnect()
{
  syslog(LOG_DEBUG, "closing connection to %s:%hu", inet_ntoa(clientListener.sin_addr), ntohs(clientListener.sin_port));
  close(sockfd);
}
