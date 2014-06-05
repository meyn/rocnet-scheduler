#!/bin/sh
echo "hello rocnet " $1
for i in `seq 1 5`; do
	echo $i
	sleep 1
done
echo "script exiting"
