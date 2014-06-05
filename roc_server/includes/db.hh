#ifndef __DB_H__
#define __DB_H__
/*
* RoCNet
* Author: Anselm Meyn
*/

#include <sqlite3.h>
#include <functional>
#include "types.hh"
#include "hosts.hh"
#include "tests.hh"

/*
const char SQL_FKEY_ON[] = "PRAGMA foreign_keys = on;";
const char SQL_FKEY_OFF[] = "PRAGMA foreign_keys = off;";
*/
class cDB {
private:
  const char* dbFilePath;
  static const char* selectAllQuery;
  static const char* updRunInfoStmt;
  sqlite3 *db;
  sqlite3_stmt *stepHostTestMap_stmt;
  sqlite3_stmt *updRunInfo_stmt;


  // for the singleton pattern
  cDB();
  cDB(cDB const&);
  void operator=(cDB const&);
  ~cDB();

public:
  static cDB& inst()
  {
  	static cDB instance;
  	return instance;
  }

  void setDbFilePath(const char* filePath);
  bool open();
  bool close();
  void stepTestHostMap(std::function<void(sHostEntry, sHostIfEntry)> cbAddHost,
                       std::function<void(UINT, sTestEntry)> cbAddTest);
  void updateRowHostRunInfo(UINT id, UINT lastRunTime);
};

#endif /* __DB_H__ */
