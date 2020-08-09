#!/bin/sh
echo "Content-type:application/xml;charset=utf-8"
echo

cat << EOF
<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="/mp2.3/name.xsl"?> 

 <root 
 xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
 xsi:schemaLocation="http://prosjekt.asuscomm.com /mp2.3/name.xsd">
  <message>
EOF
oifs=$IFS
IFS="&="
prev=""
key="1"

for var in ${QUERY_STRING}
do

    if [[ $key = "1" ]]; then
        
        if [[ $var = "fname" ]]; then
            prev="fname"
        fi

        if [[ $var = "lname"  ]]; then
            prev="lname"
        fi
        
        key="0"
    
    elif [[ $key == "0" ]]; then

        if [[ $prev = "fname" ]]; then
            HTTP_FNAME=$var
        fi

        if [[ $prev = "lname" ]]; then
            HTTP_LNAME=$var
        fi

        prev=""
        key="1"
    fi
done
IFS=$oifs
echo "<name>$HTTP_FNAME $HTTP_LNAME</name>"

cat << EOF
  </message>
 </root>
EOF