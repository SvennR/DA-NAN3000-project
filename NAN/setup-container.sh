#!/bin/bash

ROTFS=$PWD/unshare-container
INIT=init.sh
EXIT=exit.sh

if [[ ! -d "obj" ]]; then
    echo "Made obj directory"
    mkdir obj
fi

echo "Compiling server.."
make
if [ $? != "0" ] ; then
    echo -en "\n\n"
    echo "ERROR: \"make\" failed to build binary!"
    echo "Aborting.."
    exit 1
fi

echo "Checking if static binaries exist (jq, curl)"
if [ ! -f "bin-static/jq" ]; then
    echo "jq is missing.."
    echo "Building jq (will take some time)"
    ./make-static-jq.sh
    if [ $? != "0" ] ; then
        echo -en "\n\n"
        echo "ERROR: failed to build static jq!"
        echo "Aborting.."
        exit 1
    fi
fi

if [ ! -f "bin-static/curl" ]; then
    echo "curl is missing.."
    echo "Building curl (will take some time)"
    ./make-static-curl.sh
    if [ $? != "0" ] ; then
        echo -en "\n\n"
        echo "ERROR: failed to build static curl!"
        echo "Aborting.."
        exit 1
    fi
fi

echo "Creating unshare container in $ROTFS"

if [ ! -d $ROTFS ] ; then

    mkdir -p $ROTFS/{bin,proc,etc,var}

    mkdir $ROTFS/var/log
    cp mime.types $ROTFS/etc/
    cp -r www $ROTFS/var/
    sudo cp www/cant_touch_this* $ROTFS/var/www/
    cp -r www/cgi-bin $ROTFS/var/www/
    cp server $ROTFS/bin/
    cp bin-static/curl $ROTFS/bin/
    cp bin-static/jq $ROTFS/bin/
    cd       $ROTFS/bin/
    cp       /bin/busybox .
    for P in $(./busybox --list); do ln busybox $P; done;

    cat <<EOF > $INIT
#!bin/sh
mount -t proc none /proc 
exec /bin/sh
EOF
    cat <<EOF > $EXIT
#!/bin/sh
echo "Killing /bin/server"
killall server
#echo "Unmounting /proc filesystem"
#umount proc
echo "Done!"
echo ""
echo "Type \"exit\" to exit"
EOF

    chmod +x init.sh
    chmod +x exit.sh
fi

echo ""
echo "Don't worry about the \"permission denied\" errors.."
echo "The files got copied!"
echo "Done!"
echo ""
