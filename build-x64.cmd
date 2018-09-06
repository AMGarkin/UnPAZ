@mkdir obj\x64 2>nul
x86_64-w64-mingw32-g++.exe -O3 -c BDO.cpp -o obj/x64/BDO.o
x86_64-w64-mingw32-g++.exe -O3 -c BDOFiles.cpp -o obj/x64/BDOFiles.o -lz
x86_64-w64-mingw32-g++.exe -O3 -c IceKey.cpp -o obj/x64/IceKey.o
x86_64-w64-mingw32-g++.exe -O3 -c UnPAZ.cpp -o obj/x64/UnPAZ.o
x86_64-w64-mingw32-g++.exe -O3 -c Utility.cpp -o obj/x64/Utility.o
@mkdir bin\x64 2>nul
x86_64-w64-mingw32-g++.exe -o bin/x64/UnPAZ.exe obj/x64/BDO.o obj/x64/BDOFiles.o obj/x64/IceKey.o obj/x64/UnPAZ.o obj/x64/Utility.o -static -s -lboost_filesystem-mt -lboost_system-mt -lz
