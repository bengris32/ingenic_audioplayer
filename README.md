# audioplayer for Ingenic platform

libmp3lame (3.100) was cross compiled with toolchain `mipsel-openipc-linux-musl`,
use options `--host=mipsel-openipc-linux-musl --enable-static --disable-shared --disable-frontend --disable-gtktest`

To build the program, run:
```
$ make
```

To clean build objects, run:
```
$ make clean
```

# Usage
Usage is equivalent to audioplayer for hisi platforms, use:
```
cat audio.mp3 | audioplayer
```
