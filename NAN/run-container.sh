#!/bin/bash

ROTFS=$PWD/unshare-container

cd $ROTFS

sudo PATH=/bin unshare --fork --pid /usr/sbin/chroot . bin/init.sh
