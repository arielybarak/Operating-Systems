#!/bin/bash

# Run'pwd' in fg
pwd & 

# Wait a short time to ensure 'pwd' starts
sleep 1

# Simulate pressing CTRL+Z (SIGTSTP)
kill -TSTP %+ 


sleep 1

# Check if a stopped process exists
stopped_process=$(jobs -l | grep "Stopped")

if [[ -n "$stopped_process" ]]; then
  echo "A process was created and is stopped:"
  echo "$stopped_process"
else
  echo "No stopped process found. Check your handler implementation."
fi

