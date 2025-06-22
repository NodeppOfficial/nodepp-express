#!/bin/bash

set -e

sh build.sh

echo "Successfully built your application. Logs below are from your application..."
echo "============================================================================"
echo ""

./build/Debug/server
