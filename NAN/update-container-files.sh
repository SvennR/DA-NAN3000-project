#!/bin/bash

ROTFS=$PWD/unshare-container

echo "Updating unshare container files in $ROTFS"

cp mime.types $ROTFS/etc/
cp -r www $ROTFS/var/
sudo cp www/cant_touch_this* $ROTFS/var/www/
cp -r www/cgi-bin $ROTFS/var/www/

echo ""
echo "Don't worry about the \"permission denied\" errors.."
echo "The files got copied!"
echo ""
echo "Done!"