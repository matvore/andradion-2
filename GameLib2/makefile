# this mak file compiles Gamelib2 for release

.cpp.obj:
   c:\borland\bcc55\bin\bcc32.exe -O1 -c -w -w-8002 -Ic:\dxsdk\include;c:\borland\bcc55\include -DBORLAND;NDEBUG -U_DEBUG $*

GameLib2Release.lib:: Bob.obj color.obj color256.obj colornp.obj compactmap.obj graphics.obj logger.obj menu.obj musiclib.obj profiler.obj rawload.obj surfacelock.obj surfacelock256.obj timer.obj
   tlib GameLib2Release -+bob -+color -+color256 -+colornp -+compactmap -+graphics -+logger -+menu -+musiclib -+profiler  -+rawload -+surfacelock -+surfacelock256 -+timer















