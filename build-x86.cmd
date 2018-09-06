@mkdir obj\x86 2>nul
i686-w64-mingw32-g++.exe -O3 -c BDO.cpp -o obj/x86/BDO.o
i686-w64-mingw32-g++.exe -O3 -c BDOFiles.cpp -o obj/x86/BDOFiles.o -lz
i686-w64-mingw32-g++.exe -O3 -c IceKey.cpp -o obj/x86/IceKey.o
i686-w64-mingw32-g++.exe -O3 -c UnPAZ.cpp -o obj/x86/UnPAZ.o
i686-w64-mingw32-g++.exe -O3 -c Utility.cpp -o obj/x86/Utility.o
@mkdir bin\x86 2>nul
i686-w64-mingw32-g++.exe -o bin/x86/UnPAZ.exe obj/x86/BDO.o obj/x86/BDOFiles.o obj/x86/IceKey.o obj/x86/UnPAZ.o obj/x86/Utility.o -static -s -lboost_filesystem-mt -lboost_system-mt -lz
