#!/bin/bash

git pull
git add -A
git commit -m "$@"
git push
