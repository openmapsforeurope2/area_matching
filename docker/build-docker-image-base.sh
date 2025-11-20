#!/bin/sh
DOCKER_NAME=area_matching_base
DOCKER_TAG="latest"

docker build --no-cache -t $DOCKER_NAME:$DOCKER_TAG -f Dockerfile.base ./..