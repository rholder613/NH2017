
set VER=12
set RELEASE_DIR=..\..\..\bin\release
set SAVE_DIR=c:\onedrive\nethack\1.1.%VER%

mkdir %SAVE_DIR%
mkdir %SAVE_DIR%\x86
mkdir %SAVE_DIR%\x64

copy  %RELEASE_DIR%\win32\NethackUwa.pdb %SAVE_DIR%\x86
copy  %RELEASE_DIR%\x64\NethackUwa.pdb %SAVE_DIR%\x64
