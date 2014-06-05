PRAGMA foreign_keys = on;

-- list of test item keys
INSERT INTO 'testItems' (key, duration) VALUES ('rocnet.tcp.tpt.down', 25);
INSERT INTO 'testItems' (key, duration) VALUES ('rocnet.tcp.tpt.up', 25);
INSERT INTO 'testItems' (key, duration) VALUES ('rocnet.udp.tpt.down', 25);
INSERT INTO 'testItems' (key, duration) VALUES ('rocnet.icmp.nwPerf', 15);

-- list of hosts (Roc1 and Roc2)
INSERT INTO 'hosts' (name, rptInterval) VALUES ('Roc1', 300);
INSERT INTO 'hosts' (name, rptInterval) VALUES ('Roc2', 300);

-- each with 2 interfaces
INSERT INTO 'hostIfs' (hostId,ipv4,port) VALUES (1,'192.16.126.83', 10057);
INSERT INTO 'hostIfs' (hostId,ipv4,port) VALUES (1,'192.16.126.92', 10057);
INSERT INTO 'hostIfs' (hostId,ipv4,port) VALUES (2,'192.16.126.86', 10057);
INSERT INTO 'hostIfs' (hostId,ipv4,port) VALUES (2,'192.16.126.87', 10057);

-- now create the map of tests to host
INSERT INTO 'testHostMap' (testId, hostId) VALUES (1,1), (2,1), (3,1), (4,1);
INSERT INTO 'testHostMap' (testId, hostId) VALUES (1,2), (2,2), (3,2), (4,2);
