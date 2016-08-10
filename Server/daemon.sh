#! /bin/bash

while [ true ] 
do
	SERVER=` ps | grep  -c "./server" `
	if [ "$SERVER" -eq "1" ];then
		cd /
		./server &
		sleep 1
	else
		echo "server exist!"
		sleep 1
	fi
done
