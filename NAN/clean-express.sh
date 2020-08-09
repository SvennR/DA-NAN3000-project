# Stopping the express container
if [ $(sudo docker container ls --filter name=express --quiet) ];
then
	echo 'Stopping the express server'
	sudo docker container stop express
fi

# Deleting the express container
if [ $(sudo docker container ls --all --filter name=express --quiet) ];
then
        echo 'Deleting the express server'
	sudo docker container rm express
fi

# Deletes the express image
if [ $(sudo docker image list express-image --quiet) ];
then
	echo 'Removing the express image'
	sudo docker image rm express-image
fi

# Stopping all dockers since deamon is going to be stopped
sudo docker stop $(sudo docker ps --all --quiet)

# Killing the docker daemon process with the option user namespace
sudo pkill docker

# Starting the docker daemon with systemd (without user namespace)
sudo systemctl start docker

echo 'All done :)'
