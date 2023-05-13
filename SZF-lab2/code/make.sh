#!/bin/sh
bison -d szf.y
flex szf.l
bison -d szf.y
gcc szf_analyser.c szf.tab.c -ly -lfl -o parser
