#!/bin/bash

if [[ ! -d "$PWD/unshare-container" ]]; then
    echo "Unshare container doesnt exist!"
    echo "Doing nothing.."

else

    if [[ -d "$PWD/unshare-container/proc" ]]; then
        echo "Unmounting proc filesystem in unshare-container"
        sudo umount unshare-container/proc
    fi
    
    echo "Deleting unshare container $PWD/unshare-container/"
    # sudo because there might be files owned by root (or another user) in www/
    sudo rm -rf unshare-container
fi    

echo ""
echo "Done!"
