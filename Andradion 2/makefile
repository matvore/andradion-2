# this makefile builds Andradion 2 Release
#  to make the debug version,  use makefiled

.cpp.obj:
   bcc32.exe -O1 -c -w -Ic:\dxsdk\include;c:\borland\bcc55\include -DBORLAND;NDEBUG -U_DEBUG $*

.rc.res:
   brcc32.exe -DBORLAND -32 $*

"Andradion 2.exe":: Deeds.obj Character.obj Fire.obj Fixed.obj GammaEffects.obj Glue.obj LevelEnd.obj Net.obj Palette.obj PowerUp.obj Weather.obj winmain.obj "Andradion 2.res" sfx.res
   ilink32.exe /Gn /Tpe /aa /LC:\Borland\BCC55\Lib;C:\Borland\BCC55\Lib\psdk deeds character levelend fire fixed gammaeffects glue net palette powerup weather winmain C:\Borland\BCC55\Lib\c0w32, "Andradion 2.exe",, c:\GameLib2\GameLib2Release import32 cw32,, "Andradion 2.res" sfx.res
