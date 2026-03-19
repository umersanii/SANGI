#!/usr/bin/env bash
# stop script on error
set -e

# Check for python 3
if ! python3 --version &> /dev/null; then
  printf "\nERROR: python3 must be installed.\n"
  exit 1
fi

# Prefer an existing virtualenv. Use $VIRTUAL_ENV or common folders.
PYTHON_EXEC=""
if [ -n "${VIRTUAL_ENV:-}" ]; then
  PYTHON_EXEC="$VIRTUAL_ENV/bin/python"
elif [ -x "./venv/bin/python" ]; then
  PYTHON_EXEC="./venv/bin/python"
elif [ -x "./.venv/bin/python" ]; then
  PYTHON_EXEC="./.venv/bin/python"
fi

if [ -z "$PYTHON_EXEC" ]; then
  printf "\nNo existing virtualenv found. Creating one at ./venv...\n"
  python3 -m venv ./venv
  PYTHON_EXEC="./venv/bin/python"
  printf "Created virtualenv at ./venv. Activate with: source ./venv/bin/activate\n"
fi

printf "Using Python: %s\n" "$PYTHON_EXEC"

# Ensure pip/setuptools/wheel are available
"$PYTHON_EXEC" -m pip install --upgrade pip setuptools wheel

# Check to see if root CA file exists, download if not
if [ ! -f ./root-CA.crt ]; then
  printf "\nDownloading AWS IoT Root CA certificate from AWS...\n"
  curl https://www.amazontrust.com/repository/AmazonRootCA1.pem > root-CA.crt
fi

# Check to see if AWS Device SDK for Python exists, download if not
if [ ! -d ./aws-iot-device-sdk-python-v2 ]; then
  printf "\nCloning the AWS SDK...\n"
  git clone https://github.com/aws/aws-iot-device-sdk-python-v2.git --recursive
fi

# Install the AWS SDK into the virtualenv if missing
if ! "$PYTHON_EXEC" -c "import awsiot" &> /dev/null; then
  printf "\nInstalling AWS SDK into virtualenv...\n"
  "$PYTHON_EXEC" -m pip install ./aws-iot-device-sdk-python-v2
  result=$?
  if [ $result -ne 0 ]; then
    printf "\nERROR: Failed to install SDK into virtualenv.\n"
    exit $result
  fi
fi

# run pub/sub sample app using certificates downloaded in package
printf "\nRunning MQTT5 X.509 sample application...\n"
"$PYTHON_EXEC" aws-iot-device-sdk-python-v2/samples/mqtt/mqtt5_x509.py --endpoint a1qyuyxbvcois1-ats.iot.us-east-1.amazonaws.com --cert SANGI.cert.pem --key SANGI.private.key --client_id basicPubSub --topic sdk/test/python --count 0