## Game Dubois:

Game Dubois is a Game Boy emulator that I did as a personal project
It's not the fastest or the most accurate out there, but it does what an emulator does, emulates games.

### Building:

The project first needs configuring with the conf.sh script, then built with build.sh script. The executable is generated in the build folder with the name game-boi.
As a shorthand for configuring, building and running, the full\_run.sh script can be used.

### Running:

To emulate a rom, the rom's path needs to be specified with the "-c" command line flag. 
To additionally load with custom ram use the "-r" flag and specify the path after it. 
To start in cpu debug mode use the "-d" flag.
ex: `game\_boi -c "rom\_path" -r "custom\_ram_path" -d` (this command will open the rom at rom\_path, loaded with the ram at custom\_ram_path in debug mode)
