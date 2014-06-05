/*
* RoCNet
* Author: Anselm Meyn
*/

#include <iostream>
#include <iomanip>
#include <cstring>
#include "tests.hh"

using namespace std;

const char* cTest::resultErrStr[] = {
    "ROC_NOTSUPPORTED",
    "ROC_TIMEOUT",
    "ROC_READERROR",
    "ROC_ERROR"
};


std::ostream& operator<< (std::ostream& out, cTest& test)
{
  out << "[" << setw(2) << test.testItem.id << ','
      << setw(20) << test.testItem.key << ','
      << setw(5) << test.testItem.duration << ','
      << setw(20) << test.testItem.srvSetup << "], (" << &test << ")";
  return out;
}


cTest::cTest()
{
  this->testItem.id = 0;
  this->testItem.key.clear();
  this->testItem.duration = 0;
  this->testItem.srvSetup.clear();
  //std::cout<<"cTest::Ctor() : " << this << "," << *this << std::endl;
}


cTest::cTest(const sTestEntry& item)
{
  this->testItem.id = item.id;
  this->testItem.key = item.key;
  this->testItem.duration = item.duration;
  this->testItem.srvSetup = item.srvSetup;
  //std::cout<<"cTest::ctor1(): " << this << "," << *this << std::endl;
}


cTest& cTest::operator=(const cTest& rhs)
{
  this->testItem.id = rhs.testItem.id;
  this->testItem.key = rhs.testItem.key;
  this->testItem.duration = rhs.testItem.duration;
  this->testItem.srvSetup = rhs.testItem.srvSetup;
  return *this;
}


int cTest::operator==(const cTest &rhs) const
{
  if (this->testItem.id == rhs.testItem.id) {
    return 1;
  }
  return 0;
}


int cTest::operator<(const cTest &rhs) const
{
  return 0;
}

bool cTest::isValidResult(const char* resultStr)
{
  int i;
  //std::cout << sizeof(resultErrStr) << " isValidResult(): " << resultStr << std::endl;
  for (i=0; i<4; ++i) {
    //std::cout << "comparing " << resultStr << " == " << resultErrStr[i] << std::endl;
    if (strncmp(resultStr, resultErrStr[i], strlen(resultStr)) == 0) {
      return false;
    }
  }
  return true;
}