#!/bin/bash

echo "Killing..."
killall -QUIT --quiet -u cksmith web_server
echo "Making..."
make
echo -e "Launching server..."
./web_server "$@"
