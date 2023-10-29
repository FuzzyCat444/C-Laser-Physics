cl /Feclasergame.exe /O2 src\*.c /I"C:\SDL2-2.28.3\include" /link /SUBSYSTEM:CONSOLE /LIBPATH:"C:\SDL2-2.28.3\lib\x64" SDL2main.lib SDL2.lib shell32.lib
start clasergame.exe