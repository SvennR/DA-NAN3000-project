#!/bin/bash
echo "Cleaning container.."
./clean-container.sh

if [ $? != "0" ]; then
    echo "ERROR: failed to clean container.."
    echo "Aborting.."
    exit 1
fi

echo "Setting up new container.."
./setup-container.sh

if [ $? != "0" ]; then
    echo "ERROR: failed to setup container"
    echo "Aborting.."
    exit 1
fi

echo "Running container.."
./run-container.sh

echo "Done!"
