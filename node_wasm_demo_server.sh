#/bin/bash

# check if node is available
if ! [ -x "$(command -v node)" ]; then
    echo 'Error: node is not installed.' >&2
    exit 1
fi

# check if npm is available
if ! [ -x "$(command -v npm)" ]; then
    echo 'Error: npm is not installed.' >&2
    exit 1
fi

cd wasm/web-server-test

# run npm install silently to install all dependencies for the server if not already installed
npm install --silent

# run the server
node server.js
exit 0