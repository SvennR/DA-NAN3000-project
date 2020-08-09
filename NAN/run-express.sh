@#!/bin/bash

DOCKERFILE="Dockerfile"
HWPLATFORM=$(uname -m)

if [[ "$HWPLATFORM" == "arm"* || "$HWPLATFORM" == "aarch"* ]]; then
    echo "Detected ARM hardware"
    echo "Using arm.Dockerfile"
    DOCKERFILE="arm.Dockerfile"
fi

# Stoppin all docker containers
sudo docker stop $(sudo docker ps -aq)

# Killing all docker processes - daemon will start after this command if it is started on boot (systemctl enable docker)
sudo pkill docker

# Stopping the docker daemon
sudo systemctl stop docker

# Starting the docker daemon with user namespace - creates a user&group named dockremap
sudo dockerd --userns-remap="default" &

# Stopping the named container if running
if [ $(docker container ls -q --filter name=express) ];
then
    echo 'Stopping the express container...'
    sudo docker container stop express
fi

# Removing the named container if exist
if [ $(sudo docker container ls -a -f name=express -q) ];
then
    echo 'Removing the express container...'
    sudo docker container rm express
fi

# Removing named image if exist
if [ $(docker image list express-image -q) ];
then
    echo 'Removing the express image...'
    sudo docker image rm express-image
fi

cd express

echo 'Building the image...'
sudo docker build -f "$DOCKERFILE" -t express-image .


echo 'Starting the container...'

# Run the container
# -it: --interactive + --tty
# --name : Defining a name to the container - easier identify the container later on
# --cap : Remove and set capabilites to the container.
# --pids-limit : Limit possible processes by the container - prevent fork bomb
# --cpuset-cpus : Limit the specific CPUs or cores a container can use
# --cpu-shares : It prioritizes container CPU resources for the available CPU cycles
# --memory: Maximum amount of memory the container can use. m = mb
# --publish : Firewall rule which maps a container port to a port on the Docker host
sudo docker run -it \
--name express \
--cap-drop=all \
--pids-limit 200 \
--cpuset-cpus 0 \
--cpu-shares 512 \
--memory 512m \
--publish 6101:6101 \
express-image
