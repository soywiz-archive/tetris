@echo off
cls
del tetris.exe > NUL 2> NUL
call make.bat
tetris.exe
echo %ERRORLEVEL%
