@echo off

if exist ..\..\bin\Debug\data\shaders (
  rmdir /Q /S ..\..\bin\Debug\data\shaders
)

if exist ..\..\bin\Release\data\shaders (
  rmdir /Q /S ..\..\bin\Release\data\shaders
)

if exist ..\..\bin\Debug\data (
  mklink /D ..\..\bin\Debug\data\shaders ..\..\..\data\shaders
)

if exist ..\..\bin\Release\data (
  mklink /D ..\..\bin\Release\data\shaders ..\..\..\data\shaders
)
