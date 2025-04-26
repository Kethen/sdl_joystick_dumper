### Simple sdl joystick dumper

For having a quick look at what your joystick is doing on sdl

#### Building
```
# opensuse
zypper install SDL3-devel libSDL3-0 gcc-c++

# fedora 
dnf install SDL3 SDL3-devel gcc-c++

# debian trixie+
apt update; apt install libsdl3-dev libsdl3-0 g++

bash build.sh
```

#### Usage
```
./sdl_joystick_dumper
```
