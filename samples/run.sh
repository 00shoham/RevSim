#!/bin/sh

# you have to do a 'make install' from src before this will work.

rev-sim -c config.ini -o output.txt -e events.txt -k counters.txt
