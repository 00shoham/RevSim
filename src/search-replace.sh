#!/bin/sh

OLD='DebugCalloc'
NEW='SafeCalloc'

for f in `\ls *.[ch]`; do
  cat $f | sed "s/$OLD/$NEW/g" > $f.bak\
    && mv $f.bak $f
done
