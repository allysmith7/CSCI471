#!/bin/bash

echo "Making..."
make
echo -e "Launching server..."
./web_server -v &

