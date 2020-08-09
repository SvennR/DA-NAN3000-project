#!/bin/sh

# Skriver ut 'http-header' for 'plain-text'
#echo "Content-type: text/plain;charset=utf-8"
echo "Content-Type: text/plain"
# Skriver ut tom linje for å skille hodet fra kroppen
echo
# Skriver ut 'http-body'
env | sort
echo "----------------------------------------------"
echo "HELLO from miljo.cgi"
echo "QUERY_STRING ER: $QUERY_STRING"
