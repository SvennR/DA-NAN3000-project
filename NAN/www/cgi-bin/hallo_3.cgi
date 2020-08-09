#!/bin/sh

echo "Content-type: text/xml"
echo

cat << EOF
<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet href="/xmlstyle.css"?>
<?xml-stylesheet href="/hallo_3.xsl"?>
<message>
    <text> $QUERY_STRING </text>
</message>
EOF


# while IFS='&' read -ra ADDR; do for i in "${ADDR[@]}"; do echo "<text> $i </text>" done done <<< "$QUERY_STRING"

