#!/bin/bash

g++ pixel.cpp lodepng.cpp console.linux.cpp gui.tests.cpp -I. -lncurses -o gui.tests.linux -w -fpermissive -ggdb $@ 2> out.txt
if [[ $? != 0 ]]
	then
	less out.txt
else
	echo gui.tests.cpp -\> gui.tests.linux
	fi

exit 0
