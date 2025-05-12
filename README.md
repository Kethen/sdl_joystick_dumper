### Simple sdl joystick dumper

For having a quick look at what your joystick is doing on sdl

#### Building
```
# opensuse
zypper install SDL3-devel libSDL3-0 gcc-c++ mingw64-cross-gcc-c++

# fedora 
dnf install SDL3 SDL3-devel gcc-c++ mingw64-gcc-c++

# debian trixie+
apt update; apt install libsdl3-dev libsdl3-0 g++ g++-mingw-w64

bash build.sh
bash build_windows.sh
```

#### Usage
```
# linux
./sdl_joystick_dumper
```

On windows, copy sdl3_prebuilt/SDL3.dll next to sdl_joystick_dumper.exe, then double click sdl_joystick_dumper.exe
