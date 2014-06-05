/*
* RoCNet
* Author: Anselm Meyn
*/

#include <iostream>
#include "db.hh"

using namespace std;


const char* cDB::selectAllQuery = "SELECT h.id,h.name,h.rptInterval,hRun.lastStart,hIf.id,hIf.ipv4,hIf.dns,hIf.port,t.id,t.key,t.duration,t.srvSetup "
                                  "FROM testHostMap AS map "
                                  "INNER JOIN hosts AS h ON map.hostId=h.id "
                                  "INNER JOIN hostIfs AS hIf ON map.hostId=hIf.hostId "
                                  "LEFT OUTER JOIN hostRunInfo AS hRun ON map.hostId=hRun.hostId "
                                  "INNER JOIN testItems as t ON map.testId=t.id;";
const char* cDB::updRunInfoStmt = "REPLACE INTO hostRunInfo (hostId,lastStart) VALUES(?1,?2);";

cDB::cDB()
{
  dbFilePath = NULL;
  db = NULL;
}


void cDB::setDbFilePath(const char* filePath)
{
  dbFilePath = filePath;
}

cDB::~cDB()
{
  sqlite3_finalize(updRunInfo_stmt);
  close();
}


bool cDB::open()
{
  const char **pzTail = 0;

  if (!dbFilePath) {
    return false;
  }

  if (SQLITE_OK != sqlite3_open_v2(dbFilePath, &db, SQLITE_OPEN_READWRITE, NULL)) {
  	std::cerr << "Error opening database [" << dbFilePath << "]: " << sqlite3_errmsg(db) << std::endl;
  	sqlite3_close(db);
  	return false;
  }

  if (SQLITE_OK != sqlite3_prepare_v2(db, selectAllQuery, -1, &stepHostTestMap_stmt, pzTail)) {
    std::cerr << "Error sqlite3_prepare_v2: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_close(db);
    return false;
  }
  
  if (SQLITE_OK != sqlite3_prepare_v2(db, updRunInfoStmt, -1, &updRunInfo_stmt, pzTail)) {
    std::cerr << "Error sqlite3_prepare_v2: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stepHostTestMap_stmt);
    sqlite3_close(db);
    return false;
  }

  return true;
}


bool cDB::close()
{
  return (SQLITE_OK == sqlite3_close_v2(db));
}


void cDB::stepTestHostMap(std::function<void(sHostEntry, sHostIfEntry)> cbAddHost,
                          std::function<void(UINT, sTestEntry)> cbAddTest)
{ 
  sHostEntry host;
  sHostIfEntry hostIf;
  sTestEntry testItem;
  const UCHAR *tmpStr;

  while (SQLITE_ROW == sqlite3_step(stepHostTestMap_stmt)) {
    /* Create the host, hostIf and testItem entries from the result row */
    // hostEntry information
    host.id           = sqlite3_column_int(stepHostTestMap_stmt, 0);
    tmpStr            = sqlite3_column_text(stepHostTestMap_stmt, 1);
    if (tmpStr) {
      host.name       = std::string(reinterpret_cast<const char*>(tmpStr));
    } else {
      host.name.clear();
    }
    host.rptInterval  = sqlite3_column_int(stepHostTestMap_stmt, 2);
    host.lastRun      = sqlite3_column_int(stepHostTestMap_stmt, 3);

    // hostIf information
    hostIf.id         = sqlite3_column_int(stepHostTestMap_stmt, 4);
    tmpStr            = sqlite3_column_text(stepHostTestMap_stmt, 5);
    if (tmpStr) {
      hostIf.ipv4Addr = std::string(reinterpret_cast<const char*>(tmpStr));
    } else {
      hostIf.ipv4Addr.clear();
    }
    tmpStr            = sqlite3_column_text(stepHostTestMap_stmt, 6);
    if (tmpStr) {
      hostIf.dns      = std::string(reinterpret_cast<const char*>(tmpStr));
    } else {
      hostIf.dns.clear();
    }
    hostIf.port       = sqlite3_column_int(stepHostTestMap_stmt, 7);
    // and pass it to the addHost callback function
    cbAddHost(host, hostIf);

    // testItem entry information
    testItem.id       = sqlite3_column_int(stepHostTestMap_stmt, 8);
    tmpStr            = sqlite3_column_text(stepHostTestMap_stmt, 9);
    if (tmpStr) {
      testItem.key  = std::string(reinterpret_cast<const char*>(tmpStr));
    } else {
      testItem.key.clear();
    }
    testItem.duration = sqlite3_column_int(stepHostTestMap_stmt, 10);
    tmpStr            = sqlite3_column_text(stepHostTestMap_stmt, 11);
    if (tmpStr) {
      testItem.srvSetup = std::string(reinterpret_cast<const char*>(tmpStr));
    } else {
      testItem.srvSetup.clear();
    }
    // and pass it on to the add testItem callback function
    cbAddTest(host.id, testItem);
    /*std::cout << host.id << ',' << host.name << ',' << host.ipv4Addr << ',' << host.dns << ',' << host.port << ',' << host.rptInterval << ',' << host.lastRun
                << " -- " << testItem.id << ',' << testItem.key << ',' << testItem.duration << ',' << testItem.srvSetup << std::endl;*/
  }
  sqlite3_finalize(stepHostTestMap_stmt);
}


void cDB::updateRowHostRunInfo(unsigned int id, unsigned int lastRunTime)
{
  if (SQLITE_OK != sqlite3_bind_int(updRunInfo_stmt, 1, id)) {
    std::cerr << "sqlite3_bind_int() error: " << sqlite3_errmsg(db) << std::endl;
  }
  if (SQLITE_OK != sqlite3_bind_int(updRunInfo_stmt, 2, lastRunTime)) {
    std::cerr << "sqlite3_bind_int() error: " << sqlite3_errmsg(db) << std::endl;
  }

  if (SQLITE_DONE != sqlite3_step(updRunInfo_stmt)) {
    std::cerr << "sqlite3_step() error: " << sqlite3_errmsg(db) << std::endl;
  }
  sqlite3_reset(updRunInfo_stmt);
}
