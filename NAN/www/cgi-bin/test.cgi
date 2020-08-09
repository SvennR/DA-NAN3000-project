#!/bin/sh

echo Content-type: text/plain
echo

ls -l /proc/$$/fd/ >&2

if KROPP=$(timeout 3 head -c $CONTENT_LENGTH)
then
    echo Innholdet av mottatt kropp: \"$KROPP\"
else
    echo Tidsavbrudd
fi
