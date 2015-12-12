#!/bin/sh

QTBIN=/opt/local/libexec/qt5/bin

rm -rf build CellVision.app Makefile .qmake.stash

$QTBIN/qmake
make -j4

mkdir -p CellVision.app/Contents/MacOS
mkdir -p CellVision.app/Contents/Resources

cp platform/mac/Info.plist CellVision.app/Contents
cp -R data CellVision.app/Contents/MacOS
cp platform/mac/cellvision.icns CellVision.app/Contents/Resources
cp bin/cellvision CellVision.app/Contents/MacOS

#mkdir -p CellVision.app/Contents/Libs
#platform/mac/dylibbundler -od -b -x ./CellVision.app/Contents/MacOS/cellvision -d ./CellVision.app/Contents/Libs/ -p @executable_path/../Libs/
#chmod a+x CellVision.app/Contents/Libs/*

$QTBIN/macdeployqt CellVision.app -verbose=2

otool -L CellVision.app/Contents/MacOs/cellvision
