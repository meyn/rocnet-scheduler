#!/bin/sh

# RocNet linux ICMP network performance measurement script (uses ping)
# Author(s): Feidias Moulianitakis, Anselm Meyn
#
# Required arguments: IP of the server, path to Zabbix configuration file
# The script sends multiple pings and calculates the roundtrip time,
# standard deviation and error rate.

# defaults
PKT_LOSS=100 #100% packet loss
INFINITY=60000 #1 minute in ms
NUM_PINGS=10
KEY_1=rocnet.icmp.err
KEY_2=rocnet.icmp.rtt
KEY_3=rocnet.icmp.jitter
#TIMESTAMP=${2:-$(date +'%s')}

# Zabbix vars
ZBX_SEND=/opt/zabbix/bin/zabbix_sender #/usr/local/bin/zabbix_sender

PING_CMD=/sbin/ping #/bin/ping
CUT_CMD=/bin/cut #/usr/bin/cut
ECHO_CMD=/bin/echo
TAIL_CMD=/bin/tail #/usr/bin/tail

# ensure we were passed in the server ip (first argument)
# and the zabbix_sender is available
[ -z "$1" ] || [ ! -r "$2" ] || [ ! -x "$ZBX_SEND" ] && exit 1

ZBX_CONF="$2"

function handle_exit {
  $ZBX_SEND -c $ZBX_CONF -k $KEY_1 -o $1 > /dev/null
  $ZBX_SEND -c $ZBX_CONF -k $KEY_2 -o $2 > /dev/null
  $ZBX_SEND -c $ZBX_CONF -k $KEY_3 -o $3 > /dev/null
  echo $1 $2 $3
  exit $4
}

# handle interruption by sending the UNREACHABLE value
trap "handle_exit $PKT_LOSS $INFINITY $INFINITY 1" SIGINT SIGTERM

# ping the server
PING_OP=$($PING_CMD -q -i 0.2 -s 1000 -c $NUM_PINGS $1 | $TAIL_CMD -2)

# determine packet loss percent
ERR=$($ECHO_CMD $PING_OP | $CUT_CMD -d' ' -f 6 | $CUT_CMD -d'%' -f 1)

# if we have 100% packet loss then set the RTT and JITTER to INFINITY
if [ $ERR -eq 100 ]; then
  RTT=$INFINITY
  JITTER=$INFINITY
else
  RTT=$($ECHO_CMD $PING_OP | $CUT_CMD -d' ' -f 14 | $CUT_CMD -d'/' -f 2)
  JITTER=$($ECHO_CMD $PING_OP | $CUT_CMD -d' ' -f 14 | $CUT_CMD -d'/' -f 4)
fi

# send out the values to the Zabbix server
handle_exit $ERR $RTT $JITTER 0