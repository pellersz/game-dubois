#! /bin/bash

./conf.sh
./build.sh
if (( $# == 0 ))
then 
    echo "Need at least one argument"
    exit 1
elif (( $# == 1 ))
then ./build/game_boi $1
else ./build/game_boi $1 1
fi

