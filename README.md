# Audacious-Plugin-VGS
VGS BGM/MML decoder plugin for Audacious media player.

### License & Credits
All files in `bgm_decoder` directory were written by [Yoji Suzuki](https://github.com/suzukiplan). They are published with BSD 2-Clause License.

The library itself uses MIT License.

### Supported features
-   Playback `.vgs` `.bgm` `.mml`
-   Seek
-   Read metadata

### Build dependencies
-   C++11
-   CMake
-   Audacious and its devel package: Version 3.8 or higher


### How to build
```shell script
mkdir build && cd build
cmake ..
make -j `nproc`
```

### Install
Copy the `vgsmml.so` to your input plugins directory (e.g. `/usr/lib/x86_64-linux-gnu/audacious/Input/`).

Restart Audacious and enable it in settings panel.

### History
This was an amateur project when I was at school years ago.

Recently I found this code folder in a bunch of old files, and decided to make it live again.

### Screenshot
![image](https://user-images.githubusercontent.com/34613827/73605778-78808900-45dd-11ea-9b07-098a0e6fd87d.png)
