Provided .cmd files will compile source code using Visual C++ or MinGW-w64 (GCC) with boost libraries.


### How to install GCC compiler with precompiled boost libraries:
https://github.com/orlp/dev-on-windows/wiki/Installing-GCC--&-MSYS2

### Visual Studio Community (VC++):
https://www.visualstudio.com/vs/community/

### If you want use boost libraries with Visual Studio:
Homepage: http://www.boost.org/<br>
Instructions: http://www.boost.org/doc/libs/release/more/getting_started/windows.html
- Download the latest version (e.g. boost_1_65_1.zip)
- Extract it somewhere (e.g. C:\Program Files\boost\)
- Run as Administrator: Start > Visual Studio ???? > x64 Native Tools Command Prompt for VS ????
- change directory to boost root folder (cd "C:\Program Files\boost\boost_1_65_1")
- run command bootstrap
- run command .\b2
Everything is prepared, now just set in your project settings:
- C/C++ > General > Additional Include Directories to "C:\Program Files\boost\boost_1_65_1\"
- Linker > Additional Library Directories to "C:\Program Files\boost\boost_1_65_1\stage\lib\"
