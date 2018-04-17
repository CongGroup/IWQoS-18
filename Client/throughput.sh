#!/bin/bash

if [ ! $# == 5 ]; then

echo "Usage : ./Throuthput.sh [LOOP] [HeaderFile] [Type] [Time] [MiddleBox]"
echo "[Type] => 1: normal 2:setBit 4:setState."

exit
fi

BEGIN=`date +%s --date "10 seconds"`

LOOP=$1
HeaderFile=$2
Type=$3
Time=$4
MiddleBox=$5


RANDTIME=`date +%N`
RANDTIME=${RANDTIME:0:5}


get_char()
{
        SAVEDSTTY=`stty -g`
        stty -echo
        stty raw
        dd if=/dev/tty bs=1 count=1 2> /dev/null
        stty -raw
        stty echo
        stty $SAVEDSTTY
}

rm -f Output_throughput

for i in $(seq 1 ${LOOP})
do

echo "./ClientThroughput $HeaderFile  $Type $BEGIN $Time ${RANDTIME}$i $MiddleBox"
./ClientThroughput $HeaderFile $Type $BEGIN $Time ${RANDTIME}$i $MiddleBox >> Output_throughput &

done

echo "Press any key to continue..."
char=`get_char`

awk 'BEGIN{c=0;total=0}{c+=1;total+=$1}END{print c " " total}' Output_throughput



