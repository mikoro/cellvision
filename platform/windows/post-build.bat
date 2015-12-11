@echo off

if not exist bin\%1\data (
  xcopy /E /I /Y /Q data bin\%1\data
)
