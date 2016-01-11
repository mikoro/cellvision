# CellVision

A 3D volume image visualizer.

* Author: [Mikko Ronkainen](http://mikkoronkainen.com)
* Website: [github.com/mikoro/cellvision](https://github.com/mikoro/cellvision)

## Download

| Windows 64-bit                                                                                                     | Mac OS X (10.9+)                                                                                                   | Linux 64-bit (Ubuntu 15.10)                                                                                      |
|--------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------|
| [cellvision-win-0.2.1.zip](https://github.com/mikoro/cellvision/releases/download/v0.2.1/cellvision-win-0.2.1.zip) | [cellvision-mac-0.2.1.zip](https://github.com/mikoro/cellvision/releases/download/v0.2.1/cellvision-mac-0.2.1.zip) | N/A |

## Features

- Multiplatform (Windows, Linux and Mac OS X)
- Image data can be loaded from multipage TIFF image files
- Image metadata can be loaded from external files
- Volume image is visualized with a plane that intersects the volume
- Multiple different images (channels) can be visualized at the same time
- Multiple ways to move and control the camera
- Measurement tool that can measure real world distances inside the image
- Useful visual aids that help understand orientation in the world

## Instructions

Browse for the image file from the UI. Then either input manually the image specifications or browse for a metadata
file and automatically load all the values from it. Enable some or all of the color channels and select which image channel's
data will be visualized with which color. Then clicking *Load windowed/fullscreen* will update the visualization view.

### Controls

| Key                     | Action                                                                                |
|-------------------------|---------------------------------------------------------------------------------------|
| **WASD or arrows**      | Move camera around (+ Q/E for up/down)                                                |
| **Mouse left**          | Rotate camera                                                                         |
| **Mouse right**         | Orbit camera around a selected point                                                  |
| **Mouse middle**        | Measure distances                                                                     |
| **Mouse wheel**         | Move camera towards/away from the intersection plane                                  |
| **Mouse left + right**  | Move camera forwards/backwards                                                        |
| **Esc**                 | Close the fullscreen view or close the program when windowed                          |
| **F**                   | Show/hide the bottom settings panel                                                   |
| **R**                   | Reset the camera                                                                      |
| **C**                   | Show/hide coordinates                                                                 |
| **B**                   | Show/hide background                                                                  |
| **Shift**               | Move faster                                                                           |
| **Ctrl**                | Move slower                                                                           |
| **U/J**                 | Increase/decrease move speed                                                          |
| **I/K**                 | Increase/decrease mouse wheel step size                                               |
| **O/L**                 | Increase/decrease mouse speed                                                         |

## Build

### Windows

- Install Visual Studio 2013 (Community works)
- Download and install Qt 5.5 for Windows 64-bit ([https://www.qt.io/download-open-source/](https://www.qt.io/download-open-source/))
- Download and install Visual Studio Add-in 1.2.4 for Qt5 ([https://www.qt.io/download-open-source/](https://www.qt.io/download-open-source/))
- Add Qt installation path to the VS Add-in
- Add Qt installation bin directory to the path
- Open the solution and build with Visual Studio 2013
- Binaries will output to the bin directory

### Mac

- Install Xcode 7 + Command Line Tools ([https://developer.apple.com/downloads/](https://developer.apple.com/downloads/))
- Install MacPorts ([https://www.macports.org/install.php](https://www.macports.org/install.php))
  - Follow the instructions and install to /opt/local
- Install qt5 from MacPorts (```sudo port install qt5```)
- Install tiff from MacPorts (```sudo port install tiff```)
- Compile and run:

    ```
    /opt/local/libexec/qt5/bin/qmake
	make -j4
	cd bin & ./cellvision
    ```
- Build app bundle (optional):

    ```
    platform/mac/build_bundle.sh
    ```

## License

    CellVision
    Copyright © 2016 Mikko Ronkainen
    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
