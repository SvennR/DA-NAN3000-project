#!/bin/bash
clear
echo "Checking server LOG_FILE"
tail -f unshare-container/var/log/debug.log
echo Done!
