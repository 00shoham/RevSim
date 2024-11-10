#!/bin/sh

gdb --args rev-sim -c config-multi-product.ini -o /tmp/output.txt -e /tmp/events.txt -k /tmp/counters.txt -s /tmp/daily-cash.txt -u /tmp/units.txt
