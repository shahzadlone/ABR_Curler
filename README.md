# ABR Curler 
Curls and Checks any MPEG-DASH(Dynamic Adaptive Streaming over HTTP) and HLS(HTTP Live Streaming) URLs to see if they are up or not.

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
          [Path-To-File-With-The-URLS]: path to the file that contains all the URLs to the MPDs and M3Us on seperate lines.

------------------------
Examples:
------------------------
    [ ./curler URLs ]
    [ ./curler URLs 2>/dev/null ] (Try this for a cleaner output)

Note: If curling takes more than 5 seconds for any URL, program shows it's down. (Easily Configurable)

![Sample Output](/image/Example.PNG?raw=true)

------------------------
Extra:
------------------------
- Add ```2>/dev/null``` to omit the std error output and get a cleaner output.
- Use ```sort URLs | uniq --count``` to find if there are duplicates.

------------------------
FAQ:
------------------------
- Q1) If Compilation Gives Some RTMP Linker Errors.
    Try ```sudo ln -s /usr/lib/x86_64-linux-gnu/librtmp.so.1 /usr/lib/x86_64-linux-gnu/librtmp.so.0```

