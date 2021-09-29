Kumir 2.x programming system
============================

Fork features
-------------
- HTTP async requests
- Dynamic objects
- Vcpkg support
- VS Code build support
- Fixed build on Windows

***NOTE*** Maintainers from AltLinux please read this: [MAINTAINERS_ru.md](MAINTAINERS_ru.md) (in Russian).

Branches and tags
-----------------

Branch `master` contains Russian Algorithmic language implementation and all 
system modules, even not included in release.

Branch `python` contains Python-3 language implementation.

Releases are maked by tags and lists of excluded modules.
Run `remove-unstable-modules-v.X.X.X.sh` before bundling source package.

Build requirements (Linux)
--------------------------

1. CMake version at least 2.8.11
2. Python interpreter version at least 2.7.0
3. Qt4 SDK version at least 4.7.0. It is possible to build using Qt5 (>= 5.3.0)
4. ZLib development files
5. Boost 1.54.0 development files. Required Boost files are bundled into
this repository, but you can use your distribution provided package by 
deleting `src/3rdparty/boost-1.54.0`

Build requirements (Windows)
----------------------------

1. CMake version 2.8.11. There is known regression in version 2.8.12, so do
not use it
2. Python interpreter version at least 3
3. Microsoft Visual Studio 2019 Community
4. Qt5 installed with `vcpkg`
5. Boost and ZLib development files which bundled in this repository

Install libraries with vcpkg
----------------------------

1. Download and install [vcpkg](https://github.com/microsoft/vcpkg)
2. Install user-wide intergation `vcpkg integrate install`
3. Install Qt5 libraries `vcpkg install qt5:TRIPLET qt5-script:TRIPLET`
Triplet is `platform-os` pair, example `x64-windows`

Build and source-install instructions (Linux)
---------------------------------------------

Meet all requirements (see above)

Run cmake in separate directory: 
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
```

If you want to use Qt5 instead of Qt4, pass `-DUSE_QT=5` option to cmake.

Run make:
```
make
```
This will build entire project into `build/Release` directory.

To install in `/usr/local/` prefix run `make install`.

In order to provide custom prefix, you can pass option
`-DCMAKE_INSTALL_PREFIX=` to cmake.

Build instructions (Windows), w/o VS Code
----------------------------

Meet all requirements and install all libraries and tools.

Start Microsoft Visual Stido Tools console.

Within the console create subdirectory `build` of project root, walk there and
run:
```
cmake -DUSE_QT=5 -DCMAKE_BUILD_TYPE=RelWithDebInfo -Wno-dev -DBUILD_TRIPLET=TRIPLET -G "NMake Makefiles" -B . -S ..
nmake
```

Where `TRIPLET` is target triplet

This will build entire project into `build/` directory.

Build instructions (Windows), w/o VS Code
----------------------------

Meet all requirements and install all libraries and tools.

Start Microsoft Visual Stido Code.

Press `Ctrl + Shift + B`

This will build entire project into `build/` directory.

Deployment 3-rd party libraries (Windows)
-----------------------------------------

The following files must be copied into `bin` build subdirectory:
```
Qt5Core.dll
Qt5Sql.dll
Qt5Gui.dll
Qt5Network.dll
Qt5OpenGL.dll
Qt5Script.dll
Qt5Svg.dll
Qt5Xml.dll
```

