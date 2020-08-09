#!/bin/bash

if [ $# -ne 1 ] ; then
	echo "This script will connect to localhost:$TJENERPORT"
	echo "	- Request the specified file"
	echo "	- Display the response from the webserver"
	echo ""
	echo "Usage: $0 <filename>"
	exit 1;
fi

echo -en "GET /$1 HTTP/1.1\nHost: localhost\n\n" | nc localhost $TJENERPORT
