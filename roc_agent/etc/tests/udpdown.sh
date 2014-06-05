#!/bin/sh

# RocNet linux UDP download throughput measurement script (using netperf)
# Author(s): Feidias Moulianitakis, Anselm Meyn
#
# Required arguments: IP of the server, path to Zabbix configuration file

# defaults
UNREACHABLE=0
KEY=rocnet.udp.tpt.down

# zabbix vars
ZBX_SEND=/opt/zabbix/bin/zabbix_sender #/usr/local/bin/zabbix_sender

# commands
NETPERF_CMD=/usr/bin/netperf

# ensure we have a destination server (first argument)
# a readable zabbix config file
# and the zabbix_sender
[ -z "$1" ] || [ ! -r "$2" ] || [ ! -x "$ZBX_SEND" ] && exit 1

ZBX_CONF="$2"

function handle_exit {
  $ZBX_SEND -c $ZBX_CONF -k $KEY -o $1 > /dev/null
  echo $1
  exit $2
}

# handle interruption by sending the UNREACHABLE value
trap "handle_exit $UNREACHABLE 1" SIGINT SIGTERM

coproc NWTEST {
  TEST_OP=$($NETPERF_CMD -H $1 -t UDP_STREAM -P 0 -v 0 -f m -l 10 -- -m 1000)
  [ $? -eq 0 ] && echo $TEST_OP || echo $UNREACHABLE
}

read BW <& ${NWTEST[0] $1}

handle_exit $BW 0
