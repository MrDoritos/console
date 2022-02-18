::@echo on

g++ -c -DDLLEXPORT console.windows.cpp -I. -o console.windows.o

g++ -shared -o console.windows.dll console.windows.o -Wl,--output-def,console.windows.def,--out-implib,libconsole_dll.a

::"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\vsdevcmd.bat"

"C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Tools\MSVC\14.16.27023\bin\Hostx64\x64\lib.exe" /machine:amd64 /def:console.windows.def

::"C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Tools\MSVC\14.16.27023\bin\Hostx64\x64\cl.exe" console.windows.cpp console.lib -I.

::"C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Tools\MSVC\14.16.27023\bin\Hostx64\x64\cl.exe" console.windows.cpp console.lib -I. /D DLLEXPORT /D __WIN32 /EHsc 

"C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Tools\MSVC\14.16.27023\bin\Hostx64\x64\cl.exe" /LD console.windows.cpp console.windows.lib -I. /D DLLEXPORT /D __WIN32 /EHsc
