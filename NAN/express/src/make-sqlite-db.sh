#!/bin/sh

SQLDB="dikt.db"
SQLTABLES="tables.sql"
SQLROWS="rows.sql"

echo "Create sqlite database \"${SQLDB}\"" 
echo "  - with tables from   \"${SQLTABLES}\""
echo "  - with rows from     \"${SQLROWS}\""
echo

if [ ! -f "${SQLTABLES}" ]; then
    echo "ERROR: cannot find file \"${SQLTABLES}\""
    echo "Exiting.."
    exit 1
fi

if [ ! -f "${SQLROWS}" ]; then
    echo "ERROR: cannot find file \"${SQLROWS}\""
    echo "Exiting.."
    exit 1
fi

echo "Creating database in ${SQLDB}"
sqlite3 "${SQLDB}" < "${SQLTABLES}"
echo "Inserting rows.."
sqlite3 "${SQLDB}" < "${SQLROWS}"
echo "Database created"
echo "Done!"
