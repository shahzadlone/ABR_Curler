"# MPEG-DASH Curler (Check if MPDs are Up)" 
========================

------------------------
Builds:
------------------------
```sh
gcc Curler.c -Llibs/ -Ithird_party -lxml2 -lcurl -o curler -Wl,-rpath=libs/ && echo $?
```
------------------------
Usage:
------------------------
```sh
Takes one argument: [Path-To-File-With-The-URLS]
```
    WHERE:
          [Path-To-File-With-The-URLS]: path to the file that contains all the URLs to the MPDs on seperate lines.


------------------------
Examples:
------------------------
    ./curler URL_L

Note: If curling takes more than 5 seconds for any URL, program shows it's down. (Easily Configurable)