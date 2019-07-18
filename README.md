# bytecoin-gui

[![Build Status](https://dev.azure.com/bcndev/bytecoin/_apis/build/status/bytecoin-desktop?branchName=releases/3.5.1)](https://dev.azure.com/bcndev/bytecoin/_build/latest?definitionId=2&branchName=releases/3.5.1)

## How to build binaries from source code

### Windows
To build the gui you must have built bytecoin core, so please do all steps from [here](https://github.com/bcndev/bytecoin#building-on-windows) before proceed. Install [QtCreator](https://www.qt.io/download-thank-you?os=windows), open the project file bytecoin-gui/src/bytecoin-gui.pro in QtCreator and build it using MSVS kit (you must have MSVS installed already to build bytecoin core).

### MacOS

To build the gui you must have built bytecoin core, so please do all steps from [here](https://github.com/bcndev/bytecoin#building-on-mac-osx) before proceed. Install [QtCreator](https://www.qt.io/download-thank-you?os=macos), open the project file bytecoin-gui/src/bytecoin-gui.pro in QtCreator and build it using clang kit (you must have XCode installed already to build bytecoin core).

### Linux
```
# To install all required packages on Ubuntu use the following command:
$ sudo apt install qt5-qmake qtbase5-dev qtbase5-dev-tools

$ git clone https://github.com/bcndev/bytecoin.git
$ cd bytecoin
$ mkdir -p build
$ cd build
$ cmake ..
$ make -j4 bytecoin-crypto
$ cd ../..
$ git clone https://github.com/bcndev/bytecoin-gui.git
$ cd bytecoin-gui
$ mkdir -p build
$ cd build
$ cmake ..
$ make -j4
```
Alternative way:
```
# Install QtCreator:
$ sudo apt install qtcreator

$ git clone https://github.com/bcndev/bytecoin.git
$ cd bytecoin
$ mkdir -p build
$ cd build
$ cmake ..
$ make -j4 bytecoin-crypto
$ cd ../..
$ git clone https://github.com/bcndev/bytecoin-gui.git
```
Now open the project file bytecoin-gui/src/bytecoin-gui.pro in QtCreator and build it.
