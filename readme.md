# CellVision

3D image visualizer.

## Features

### Done

- Basic project structure
- Multiplatform support (Windows 64-bit and Mac OS X)
- Simple desktop UI
- OpenGL rendering widget
- Image data loading from multipage TIFF files
- 3D texture sampling and simple rendering

### Todo

- Transformation of the "view plane" in some reasonable manner
- Mac build has not been tested at all
- ???

## Build

### Windows

- Install Visual Studio 2013
- Download and install Qt 5.5 for Windows 64-bit [https://www.qt.io/download-open-source/](https://www.qt.io/download-open-source/)
- Download and install Visual Studio Add-in 1.2.4 for Qt5
- Add Qt installation path to the VS Add-in
- Add Qt installation bin directory to the path
- Open the solution and build with Visual Studio
- Binaries will output to the bin directory

### Mac

- Install Xcode 7 + Command Line Tools
- Install MacPorts
- Install qt5 (macports)
- Install tiff (macports)
- Compile and run:
    ```
	/opt/local/libexec/qt5/bin/qmake
	make -j4
	cd bin & ./cellvision
	```
- Build app bundle:
    ```
	platform/mac/build_bundle.sh
	```
