#!/bin/bash
rm -f /home/asj/lab3/test.log
for i in {1..100}
do

echo "data now : $(date +%Y)-$(date +%m)-$(date +%d) $(date +%H):$(date +%M):$(date +%S)"
NowDate='date'
echo "[$NOW] i = $i" >> /home/asj/lab3/test.log
  sleep 2
done
echo done...
