FROM fedora:42
RUN dnf install -y SDL3 SDL3-devel gcc-c++ mingw64-gcc-c++
