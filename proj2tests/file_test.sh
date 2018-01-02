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

#Open file, block it, then try to open it again
echo "Check to see if I can open the README:"
#This prints the first line of the README, if it can be opened
head -n 1 ../proj2tests/README
echo
echo "Block the README, then try to open it again:"
./fc421_block_file ../proj2tests/README
#This prints the first line of the README, if it can be opened
head -n 1 ../proj2tests/README
echo
#Print the number of times the file has been accessed(Should be at least 1)
echo "Check access count of README:"
./fc421_query ../proj2tests/README
echo
#Unlock the file, and show that it can be accessed again
echo "Unblock the readme, then try to open it again:"
./fc421_unblock_file ../proj2tests/README
#This prints the first line of the README, if it can be opened
head -n 1 ../proj2tests/README
