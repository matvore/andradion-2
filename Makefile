cc = gcc -O3 -DNDEBUG -U_DEBUG -I./DXInclude/
ccd = gcc -O3 -UNDEBUG -D_DEBUG -I./DXInclude/
out = Andradion\ 2.exe
debugout = and2debug.exe
cleaning = *.o debug\*.o "Andradion 2.exe" "and2debug.exe" "Andradion 2.res"
rm = del /Q

all: $(out) $(debugout)

levelmaker:
	rc /r utility\LevelMaker.rc
	$(cc) -c utility/LevelMaker.cpp
	windres -iutility/LevelMaker.res -oLevelMakerRes.o
	$(cc) -o LevelMaker.exe LevelMaker.o LevelMakerRes.o -lstdc++ -lgdi32
	LevelMaker.exe
	$(rm) LevelMaker.exe
	$(rm) LevelMakerRes.o
	$(rm) LevelMaker.o
	$(rm) utility\LevelMaker.res

clean:
	$(rm) $(cleaning)

cleanmacs:
	$(rm) *~
	$(rm) utility\*~

LevelsLib:
	rc LevelsLib.rc
	rc LevelsLib2.rc
	"c:\program files\Microsoft Visual Studio .NET\Vc7\bin\link.exe" /DLL /OUT:LevelsLib2.dat /NOENTRY /MACHINE:X86 LevelsLib2.res
	"c:\program files\Microsoft Visual Studio .NET\Vc7\bin\link.exe" /DLL /OUT:LevelsLib.dat /NOENTRY /MACHINE:X86 LevelsLib.res
	$(rm) LevelsLib*.res

$(out): Deeds.o Character.o Fire.o Fixed.o GammaEffects.o Glue.o LevelEnd.o Net.o Palette.o PowerUp.o Weather.o WinMain.o Andradion\ 2.o Bob.o Color.o Color256.o ColorNP.o CompactMap.o Graphics.o Menu.o MusicLib.o Profiler.o RawLoad.o SurfaceLock.o SurfaceLock256.o Timer.o
	$(cc) -o $(out) *.o -lmingw32 -lstdc++ -lwinmm -ldxguid -lgdi32 -lole32 -luuid

$(debugout): debug/Deeds.o debug/Character.o debug/Fire.o debug/Fixed.o debug/GammaEffects.o debug/Glue.o debug/LevelEnd.o debug/Net.o debug/Palette.o debug/PowerUp.o debug/Weather.o debug/WinMain.o debug/Andradion\ 2.o debug/Bob.o debug/Color.o debug/Color256.o debug/ColorNP.o debug/CompactMap.o debug/Graphics.o debug/Menu.o debug/MusicLib.o debug/Profiler.o debug/RawLoad.o debug/SurfaceLock.o debug/SurfaceLock256.o debug/Timer.o
	$(ccd) -o $(debugout) debug/*.o -lmingw32 -lstdc++ -lwinmm -ldxguid -lgdi32 -lole32 -luuid

Andradion\ 2.o: Andradion\ 2.rc
	rc /i "C:\Program Files\Microsoft SDK\Include" /r "Andradion 2.rc"
	windres -iAndradion\ 2.res -oAndradion\ 2.o
	$(rm) "Andradion 2.res"

debug/Andradion\ 2.o: Andradion\ 2.o
	rc /i "C:\Program Files\Microsoft SDK\Include" /r "Andradion 2.rc"
	windres -iAndradion\ 2.res -odebug/Andradion\ 2.o
	$(rm) "Andradion 2.res"

Deeds.o: Deeds.cpp stdafx.h fixed.h sharedconstants.h graphics.h character.h fire.h glue.h deeds.h resource.h
	$(cc) -c Deeds.cpp 

debug/Deeds.o: Deeds.o
	$(ccd) -c Deeds.cpp -o debug/Deeds.o

Character.o: Character.cpp StdAfx.h Fixed.h SharedConstants.h Graphics.h GammaEffects.h Net.h PowerUp.h Character.h Fire.h Glue.h Profiler.h Logger.h Color.h Color256.h
	$(cc) -c Character.cpp

debug/Character.o: Character.o
	$(ccd) -c Character.cpp -o debug/Character.o

Fire.o: Fire.cpp StdAfx.h Fixed.h Net.h SharedConstants.h Certifiable.h Color.h Color256.h Graphics.h SurfaceLock.h SurfaceLock256.h Character.h Fire.h Glue.h
	$(cc) -c Fire.cpp

debug/Fire.o: Fire.o
	$(ccd) -c Fire.cpp -o debug/Fire.o

Fixed.o: Fixed.cpp Fixed.h StdAfx.h
	$(cc) -c Fixed.cpp

debug/Fixed.o: Fixed.o
	$(ccd) -c Fixed.cpp -o debug/Fixed.o

GammaEffects.o: GammaEffects.cpp StdAfx.h Graphics.h Fixed.h GammaEffects.h Logger.h Palette.h Certifiable.h SurfaceLock.h SurfaceLock256.h Weather.h
	$(cc) -c GammaEffects.cpp

debug/GammaEffects.o: GammaEffects.o
	$(ccd) -c GammaEffects.cpp -o debug/GammaEffects.o

Glue.o: Glue.cpp StdAfx.h Bob.h Graphics.h Certifiable.h SurfaceLock.h SurfaceLock256.h Color.h Color256.h CompactMap.h Fixed.h SharedConstants.h Weather.h Palette.h Fire.h Character.h Glue.h Deeds.h LevelEnd.h PowerUp.h Net.h GammaEffects.h resource.h BitArray2D.h Logger.h Profiler.h MusicLib.h Timer.h Menu.h RawLoad.h
	$(cc) -c Glue.cpp

debug/Glue.o: Glue.o
	$(ccd) -c Glue.cpp -o debug/Glue.o

LevelEnd.o: LevelEnd.cpp StdAfx.h Fixed.h SharedConstants.h Graphics.h Bob.h Character.h LevelEnd.h
	$(cc) -c LevelEnd.cpp

debug/LevelEnd.o: LevelEnd.o
	$(ccd) -c LevelEnd.cpp -o debug/LevelEnd.o

Net.o: Net.cpp StdAfx.h Fixed.h SharedConstants.h Net.h Certifiable.h SurfaceLock.h SurfaceLock256.h Graphics.h Character.h Fire.h Glue.h Resource.h PowerUp.h Weather.h Logger.h
	$(cc) -c Net.cpp

debug/Net.o: Net.o
	$(ccd) -c Net.cpp -o debug/Net.o

Palette.o: Palette.cpp Fixed.h StdAfx.h SharedConstants.h Certifiable.h SurfaceLock.h SurfaceLock256.h Color.h Color256.h Graphics.h Weather.h Palette.h Fire.h Character.h Glue.h Logger.h
	$(cc) -c Palette.cpp

debug/Palette.o: Palette.o
	$(ccd) -c Palette.cpp -o debug/Palette.o

PowerUp.o: PowerUp.cpp StdAfx.h Fixed.h SharedConstants.h Graphics.h Character.h Fire.h Glue.h PowerUp.h Net.h MusicLib.h
	$(cc) -c PowerUp.cpp

debug/PowerUp.o: PowerUp.o
	$(ccd) -c PowerUp.cpp -o debug/PowerUp.o

Weather.o: Weather.cpp StdAfx.h Fixed.h SharedConstants.h Net.h Certifiable.h SurfaceLock.h SurfaceLock256.h Weather.h resource.h Character.h Fire.h Glue.h Palette.h Logger.h Profiler.h Color.h Color256.h RawLoad.h
	$(cc) -c Weather.cpp

debug/Weather.o: Weather.o
	$(ccd) -c Weather.cpp -o debug/Weather.o

WinMain.o: WinMain.cpp StdAfx.h Fixed.h SharedConstants.h Graphics.h Character.h Fire.h Glue.h resource.h Logger.h
	$(cc) -c WinMain.cpp

debug/WinMain.o: WinMain.o
	$(ccd) -c WinMain.cpp -o debug/WinMain.o

Bob.o: Bob.cpp StdAfx.h Certifiable.h Color.h ColorNP.h Color256.h Bob.h CompactMap.h Graphics.h Logger.h
	$(cc) -c Bob.cpp

debug/Bob.o: Bob.o
	$(ccd) -c Bob.cpp -o debug/Bob.o

Color.o: Color.cpp StdAfx.h Certifiable.h Color.h
	$(cc) -c Color.cpp

debug/Color.o: Color.o
	$(ccd) -c Color.cpp -o debug/Color.o

Color256.o: Color256.cpp StdAfx.h Certifiable.h Color.h Color256.h
	$(cc) -c Color256.cpp

debug/Color256.o: Color256.o
	$(ccd) -c Color256.cpp -o debug/Color256.o

ColorNP.o: ColorNP.cpp StdAfx.h Certifiable.h Color.h ColorNP.h CompactMap.h Graphics.h
	$(cc) -c ColorNP.cpp

debug/ColorNP.o: ColorNP.o
	$(ccd) -c ColorNP.cpp -o debug/ColorNP.o

CompactMap.o: CompactMap.cpp StdAfx.h Certifiable.h Color.h Color256.h ColorNP.h CompactMap.h Graphics.h Logger.h
	$(cc) -c CompactMap.cpp

debug/CompactMap.o: CompactMap.o
	$(ccd) -c CompactMap.cpp -o debug/CompactMap.o

Graphics.o: Graphics.cpp StdAfx.h  Certifiable.h SurfaceLock.h Color.h ColorNP.h Color256.h CompactMap.h Graphics.h logger.h Timer.h Bob.h
	$(cc) -c Graphics.cpp

debug/Graphics.o: Graphics.o
	$(ccd) -c Graphics.cpp -o debug/Graphics.o

Menu.o: Menu.cpp StdAfx.h Certifiable.h CompactMap.h Graphics.h Color.h Color256.h Bob.h Menu.h Logger.h
	$(cc) -c Menu.cpp

debug/Menu.o: Menu.o
	$(ccd) -c Menu.cpp -o debug/Menu.o

MusicLib.o: MusicLib.cpp StdAfx.h MusicLib.h Logger.h
	$(cc) -c MusicLib.cpp

debug/MusicLib.o: MusicLib.o
	$(ccd) -c MusicLib.cpp -o debug/MusicLib.o

Profiler.o: Profiler.cpp StdAfx.h Profiler.h
	$(cc) -c Profiler.cpp

debug/Profiler.o: Profiler.o
	$(ccd) -c Profiler.cpp -o debug/Profiler.o

RawLoad.o: RawLoad.cpp StdAfx.h
	$(cc) -c RawLoad.cpp

debug/RawLoad.o: RawLoad.o
	$(ccd) -c RawLoad.cpp -o debug/RawLoad.o

SurfaceLock.o: SurfaceLock.cpp StdAfx.h Certifiable.h Bob.h CompactMap.h Graphics.h SurfaceLock.h
	$(cc) -c SurfaceLock.cpp

debug/SurfaceLock.o: SurfaceLock.o
	$(ccd) -c SurfaceLock.cpp -o debug/SurfaceLock.o

SurfaceLock256.o: SurfaceLock256.cpp StdAfx.h Certifiable.h SurfaceLock.h SurfaceLock256.h
	$(cc) -c SurfaceLock256.cpp

debug/SurfaceLock256.o: SurfaceLock256.o
	$(ccd) -c SurfaceLock256.cpp -o debug/SurfaceLock256.o

Timer.o: Timer.cpp StdAfx.h Timer.h
	$(cc) -c Timer.cpp

debug/Timer.o: Timer.o
	$(ccd) -c Timer.cpp -o debug/Timer.o
