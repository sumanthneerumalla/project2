#!/bin/bash

#Check to make sure the user is root
if [[ $EUID -ne 0 ]]; then
	echo "Must be root to run"
	exit 1
fi

#Make the current directory proj2driver, and run make
#so it can use the driver files
echo "Make sure the driver files are compiled in proj2driver:"
cd ../proj2driver
make
echo

echo "Initial check to see if google can bind to port 80:"
nc -vz www.google.com 80
echo
echo "Check to see if google can bind to port 80 after blocking:"
./fw421_block_port tcp in 80
nc -vz www.google.com 80
echo
echo "Check access count of port 80 for incoming tcp:"
./fw421_query tcp in 80
echo
echo "Unblock port 80, then see if google can connect again:"
./fw421_unblock_port tcp in 80
nc -vz www.google.com 80

echo
echo "Try to send a UDP request to localhost:"
#Sends a UDP packet to port 8080, and prints "Success" if it works
echo -n "foo" | nc -u -w1 localhost 8080 && echo "Success"
echo
echo "Check to see if the request works after after blocking port 8080:"
./fw421_block_port udp in 8080
#Sends a UDP packet to port 8080, and prints "Success" if it works
echo -n "foo" | nc -u -w1 localhost 8080 && echo "Success"
echo
echo "Check access count of port 8080 for incoming udp:"
./fw421_query udp in 8080
echo
echo "Unblock port 8080, then see if the udp packet can go through:"
./fw421_unblock_port udp in 8080
#Sends a UDP packet to port 8080, and prints "Success" if it works
echo -n "foo" | nc -u -w1 localhost 8080 && echo "Success"