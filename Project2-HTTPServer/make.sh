#!/bin/bash

echo "Making..."
make
echo -e "\nLaunching server in background..."
./web_server -v &
echo -e "\nWaiting..."
sleep 1
echo -e "\nSending web request via curl..."
curl "http://127.0.0.1:1024/${@}"

