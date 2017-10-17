primitiv
========

A Neural Network Toolkit.


Features
--------

- Dynamic and incremental graph construction
- On-demand memory allocation
- Automatic minibatch broadcasting
- Mostly device-independent
- Simple usage


Prerequisites
-------------

- CMake 3.1.0 or later
- GCC 4.8 or later
- [Protocol Buffers](https://github.com/google/protobuf)
- (optional) [Google Test](https://github.com/google/googletest)
  - Required only when `-DPRIMITIV_BUILD_TESTS=ON`.
- (optional) CUDA 7.5 or later
  - Required only when `-DPRIMITIV_USE_CUDA=ON`

Install
-------

### Install Protocol Buffers (for Ubuntu 16.04 users) ###

The default `protobuf` repository on Ubuntu 16.04 does not support *proto3* format and
users need to install newer library from source.
Typical step to build/install `protobuf` is below:

    sudo apt install autoconf automake build-essential cmake libtool unzip
    git clone https://github.com/google/protobuf
    cd protobuf
    ./autogen.sh
    ./configure
    make [-j <threads>]
    make check
    sudo make install
    sudo ldconfig

### Install `primitiv` ###

    git clone <this repository>
    cd primitiv
    mkdir build
    cd build
    cmake .. [-DPRIMITIV_USE_CUDA=ON] [(Other options listed below if necessary)]
    make [-j <threads>]
    [make test]
    [make install]

Building Options
----------------

- `PRIMITIV_BUILD_STATIC_LIBRARY` (default=`OFF`)
  - Builds a static library instead of a shared object.
- `PRIMITIV_BUILD_TESTS` (default=`OFF`)
  - Builds test binaries and generates `make test` command.
- `GTEST_SOURCE_DIR` (default=`OFF`)
  - Specifies the source directory of Google Test. If you installed `googletest` package
    of Debian or Ubuntu, please add `-DGTEST_SOURCE_DIR=/usr/src/googletest/googletest`
    with `PRIMITIV_BUILD_TESTS` option.
- `PRIMITIV_USE_CACHE` (default=`OFF`)
  - Whether or not to use cached values to prevent increasing computation amount.
  - Libraries built with this flag will tend to consume more memory.
- `PRIMITIV_USE_CUDA` (default=`OFF`)
  - Enables CUDA backend (`devices::CUDA` class).
- Other available options:
  - CMake standard options.
  - [FindCUDA](https://cmake.org/cmake/help/v3.1/module/FindCUDA.html) options.
  - [FindProtobuf](https://cmake.org/cmake/help/v3.1/module/FindProtobuf.html) options.
  - [FindGTest](https://cmake.org/cmake/help/v3.1/module/FindGTest.html) options.

Usage
-----

See [examples](https://github.com/odashi/primitiv/tree/master/examples).


Contact
-------

- yus.takara at gmail.com
- [@odashi_t on Twitter](https://twitter.com/odashi_t)

This project is supported by [ASTREC](http://astrec.nict.go.jp/) in [NICT](http://nict.go.jp/).
