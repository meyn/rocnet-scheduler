/*
 * RoCNet
 * sqlite3 db queries
 * Author: Anselm Meyn
*/

# query used to select all the required information about hosts that we need to test
SELECT h.id,h.name,h.rptInterval,hIf.id,hIf.ipv4,hIf.port,hIf.dns,hRun.lastStart,t.id,t.key,t.duration,t.srvSetup
	FROM testHostMap AS map
  	INNER JOIN hosts AS h
  		ON map.hostId=h.id
  	INNER JOIN hostIfs AS hIf
  		ON map.hostId=hIf.hostId
	LEFT OUTER JOIN hostRunInfo AS hRun
		ON map.hostId=hRun.hostId
	INNER JOIN testItems as t
		ON map.testId=t.id;

# SQL statement to update (or create if not present) the lastStart time of a host
#REPLACE INTO hostRunInfo (hostId,lastStart) VALUES(?1,?2);