#!/bin/bash

g++ console.linux.cpp console.tests.cpp -I. -lncursesw -o console.tests.linux $@ 2> out.txt
if [[ $? != 0 ]]
	then
	less out.txt
else
	echo console.tests.cpp -\> console.tests.linux
	fi

exit 0
