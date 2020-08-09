#!/bin/sh

# Skriver ut 'http-header' for 'plain-text'
echo "Content-type: text/plain"

# Skriver ut tom linje for å skille hodet fra kroppen
echo

# Skriver ut 'http-body'
echo "Hallo $QUERY_STRING. Ha en fin dag :-)"
