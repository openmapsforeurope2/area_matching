#!/bin/sh
PROJECT_NAME=area_matching
DOCKER_TAG="latest"

docker build --no-cache -t $PROJECT_NAME:$DOCKER_TAG -f Dockerfile.dev .