#! /bin/bash

echo $#
./conf.sh
./build.sh
if (( $# == 0 ))
then ./build/game_boi
else ./build/game_boi 1
fi

