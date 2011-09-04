cc = gcc -s -O2 -DNDEBUG -U_DEBUG -I./DXInclude/
ccd = gcc -g -O2 -UNDEBUG -D_DEBUG -I./DXInclude/

out = Andradion2.exe
debugout = and2debug.exe
cleaning = *.o debug\*.o $(out) $(debugout) "Andradion 2.res"
rm = del /Q
gamebmps = "C:\Andradion 2\Resource\1charminn.bmp" "C:\Andradion 2\Resource\1charmins.bmp" "C:\Andradion 2\Resource\1charminw.bmp" "C:\Andradion 2\Resource\1cokens.bmp" "C:\Andradion 2\Resource\1cokew.bmp" "C:\Andradion 2\Resource\1evilturnern.bmp" "C:\Andradion 2\Resource\1evilturners.bmp" "C:\Andradion 2\Resource\1evilturnerw.bmp" "C:\Andradion 2\Resource\1koolaidguyn.bmp" "C:\Andradion 2\Resource\1koolaidguys.bmp" "C:\Andradion 2\Resource\1koolaidguyw.bmp" "C:\Andradion 2\Resource\1miltonn.bmp" "C:\Andradion 2\Resource\1miltons.bmp" "C:\Andradion 2\Resource\1miltonw.bmp" "C:\Andradion 2\Resource\1pepsin.bmp" "C:\Andradion 2\Resource\1pepsis.bmp" "C:\Andradion 2\Resource\1pepsiw.bmp" "C:\Andradion 2\Resource\1sallyn.bmp" "C:\Andradion 2\Resource\1sallys.bmp" "C:\Andradion 2\Resource\1sallyw.bmp" "C:\Andradion 2\Resource\1switzn.bmp" "C:\Andradion 2\Resource\1switzs.bmp" "C:\Andradion 2\Resource\1switzw.bmp" "C:\Andradion 2\Resource\1turnern.bmp" "C:\Andradion 2\Resource\1turners.bmp" "C:\Andradion 2\Resource\1turnerw.bmp" "C:\Andradion 2\Resource\2charminn.bmp" "C:\Andradion 2\Resource\2charmins.bmp" "C:\Andradion 2\Resource\2charminw.bmp" "C:\Andradion 2\Resource\2cokens.bmp" "C:\Andradion 2\Resource\2cokew.bmp" "C:\Andradion 2\Resource\2evilturnern.bmp" "C:\Andradion 2\Resource\2evilturners.bmp" "C:\Andradion 2\Resource\2evilturnerw.bmp" "C:\Andradion 2\Resource\2koolaidguyn.bmp" "C:\Andradion 2\Resource\2koolaidguys.bmp" "C:\Andradion 2\Resource\2koolaidguyw.bmp" "C:\Andradion 2\Resource\2miltonn.bmp" "C:\Andradion 2\Resource\2miltons.bmp" "C:\Andradion 2\Resource\2miltonw.bmp" "C:\Andradion 2\Resource\2pepsin.bmp" "C:\Andradion 2\Resource\2pepsis.bmp" "C:\Andradion 2\Resource\2pepsiw.bmp" "C:\Andradion 2\Resource\2sallyn.bmp" "C:\Andradion 2\Resource\2sallys.bmp" "C:\Andradion 2\Resource\2sallyw.bmp" "C:\Andradion 2\Resource\2switzn.bmp" "C:\Andradion 2\Resource\2switzs.bmp" "C:\Andradion 2\Resource\2switzw.bmp" "C:\Andradion 2\Resource\2turnern.bmp" "C:\Andradion 2\Resource\2turners.bmp" "C:\Andradion 2\Resource\2turnerw.bmp" "C:\Andradion 2\Resource\bazookan.bmp" "C:\Andradion 2\Resource\bazookas.bmp" "C:\Andradion 2\Resource\bazookaw.bmp" "C:\Andradion 2\Resource\bloodstain.bmp" "C:\Andradion 2\Resource\bullet.bmp" "C:\Andradion 2\Resource\decapitate1.bmp" "C:\Andradion 2\Resource\decapitate2.bmp" "C:\Andradion 2\Resource\explosion.bmp" "C:\Andradion 2\Resource\Health.bmp" "C:\Andradion 2\Resource\mgn.bmp" "C:\Andradion 2\Resource\mgs.bmp" "C:\Andradion 2\Resource\mgw.bmp" "C:\Andradion 2\Resource\pistoln.bmp" "C:\Andradion 2\Resource\pistols.bmp" "C:\Andradion 2\Resource\pistolw.bmp" "C:\Andradion 2\palettes\extracolors.bmp"

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

quantizepalettes:
	$(cc) utility\Quantize.cpp -lstdc++ -lgdi32
	a.exe 256 $(gamebmps) levels\1_?.bmp > palettes\1_.pal
	a.exe 256 $(gamebmps) levels\2a?.bmp > palettes\2a.pal
	a.exe 256 $(gamebmps) levels\2b?.bmp > palettes\2b.pal
	a.exe 256 $(gamebmps) levels\3a?.bmp > palettes\3a.pal
	a.exe 256 $(gamebmps) levels\3b?.bmp > palettes\3b.pal
	a.exe 256 $(gamebmps) levels\4_?.bmp > palettes\4_.pal
	a.exe 256 $(gamebmps) levels\5_?.bmp > palettes\5_.pal
	a.exe 256 $(gamebmps) levels\6a?.bmp > palettes\6a.pal
	a.exe 256 $(gamebmps) levels\6b?.bmp > palettes\6b.pal
	a.exe 256 $(gamebmps) levels\7a?.bmp > palettes\7a.pal
	a.exe 256 $(gamebmps) levels\7b?.bmp > palettes\7b.pal
	$(rm) a.exe

clean:
	$(rm) $(cleaning)

cleanmacs:
	$(rm) *~
	$(rm) utility\*~

$(out): Deeds.o Character.o Fire.o Fixed.o GammaEffects.o Glue.o LevelEnd.o Net.o Palette.o PowerUp.o Weather.o WinMain.o Andradion\ 2.o CompactMap.o Graphics.o Menu.o MusicLib.o Profiler.o RawLoad.o Timer.o
	$(cc) -o $(out) *.o -lstdc++ -lwinmm -ldxguid -lgdi32 -lole32 -mwindows -s

$(debugout): debug/Deeds.o debug/Character.o debug/Fire.o debug/Fixed.o debug/GammaEffects.o debug/Glue.o debug/LevelEnd.o debug/Net.o debug/Palette.o debug/PowerUp.o debug/Weather.o debug/WinMain.o debug/Andradion\ 2.o debug/CompactMap.o debug/Graphics.o debug/Menu.o debug/MusicLib.o debug/Profiler.o debug/RawLoad.o debug/Timer.o
	$(ccd) -o $(debugout) debug/*.o -lstdc++ -lwinmm -ldxguid -lgdi32 -lole32

Andradion\ 2.o: Andradion\ 2.rc Glue.h
	rc /i "C:\Program Files\Microsoft SDK\Include" /r "Andradion 2.rc"
	windres -iAndradion\ 2.res -oAndradion\ 2.o
	$(rm) "Andradion 2.res"

debug/Andradion\ 2.o: Andradion\ 2.o
	rc /i "C:\Program Files\Microsoft SDK\Include" /r "Andradion 2.rc"
	windres -iAndradion\ 2.res -odebug/Andradion\ 2.o
	$(rm) "Andradion 2.res"

Deeds.o: Deeds.cpp StdAfx.h Fixed.h Deeds.h
	$(cc) -c Deeds.cpp 

debug/Deeds.o: Deeds.o
	$(ccd) -c Deeds.cpp -o debug/Deeds.o

Character.o: Character.cpp StdAfx.h Fixed.h GammaEffects.h Net.h PowerUp.h Character.h Fire.h Glue.h Logger.h Graphics.h
	$(cc) -c Character.cpp

debug/Character.o: Character.o
	$(ccd) -c Character.cpp -o debug/Character.o

Fire.o: Fire.cpp StdAfx.h Fixed.h Net.h Graphics.h Character.h Fire.h Glue.h
	$(cc) -c Fire.cpp

debug/Fire.o: Fire.o
	$(ccd) -c Fire.cpp -o debug/Fire.o

Fixed.o: Fixed.cpp Fixed.h StdAfx.h
	$(cc) -c Fixed.cpp

debug/Fixed.o: Fixed.o
	$(ccd) -c Fixed.cpp -o debug/Fixed.o

GammaEffects.o: GammaEffects.cpp StdAfx.h Graphics.h Fixed.h GammaEffects.h Logger.h Palette.h 
	$(cc) -c GammaEffects.cpp

debug/GammaEffects.o: GammaEffects.o
	$(ccd) -c GammaEffects.cpp -o debug/GammaEffects.o

Glue.o: Glue.cpp StdAfx.h Graphics.h CompactMap.h Fixed.h Weather.h Fire.h Character.h Glue.h Deeds.h LevelEnd.h PowerUp.h Net.h GammaEffects.h Logger.h Profiler.h MusicLib.h Timer.h Menu.h RawLoad.h Buffer.h
	$(cc) -c Glue.cpp

debug/Glue.o: Glue.o
	$(ccd) -c Glue.cpp -o debug/Glue.o

LevelEnd.o: LevelEnd.cpp StdAfx.h Fixed.h LevelEnd.h
	$(cc) -c LevelEnd.cpp

debug/LevelEnd.o: LevelEnd.o
	$(ccd) -c LevelEnd.cpp -o debug/LevelEnd.o

Net.o: Net.cpp StdAfx.h Net.h Logger.h
	$(cc) -c Net.cpp

debug/Net.o: Net.o
	$(ccd) -c Net.cpp -o debug/Net.o

Palette.o: Palette.cpp Fixed.h StdAfx.h Graphics.h Logger.h Palette.h 
	$(cc) -c Palette.cpp

debug/Palette.o: Palette.o
	$(ccd) -c Palette.cpp -o debug/Palette.o

PowerUp.o: PowerUp.cpp StdAfx.h Fixed.h Graphics.h Character.h Glue.h PowerUp.h Net.h MusicLib.h
	$(cc) -c PowerUp.cpp

debug/PowerUp.o: PowerUp.o
	$(ccd) -c PowerUp.cpp -o debug/PowerUp.o

Weather.o: Weather.cpp StdAfx.h Fixed.h Weather.h Logger.h Graphics.h
	$(cc) -c Weather.cpp

debug/Weather.o: Weather.o
	$(ccd) -c Weather.cpp -o debug/Weather.o

WinMain.o: WinMain.cpp StdAfx.h Fixed.h Glue.h Logger.h
	$(cc) -c WinMain.cpp

debug/WinMain.o: WinMain.o
	$(ccd) -c WinMain.cpp -o debug/WinMain.o

CompactMap.o: CompactMap.cpp StdAfx.h CompactMap.h Graphics.h Logger.h Buffer.h
	$(cc) -c CompactMap.cpp

debug/CompactMap.o: CompactMap.o
	$(ccd) -c CompactMap.cpp -o debug/CompactMap.o

Graphics.o: Graphics.cpp StdAfx.h Graphics.h logger.h Buffer.h
	$(cc) -c Graphics.cpp

debug/Graphics.o: Graphics.o
	$(ccd) -c Graphics.cpp -o debug/Graphics.o

Menu.o: Menu.cpp StdAfx.h Graphics.h Menu.h Logger.h 
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

RawLoad.o: RawLoad.cpp StdAfx.h Logger.h
	$(cc) -c RawLoad.cpp

debug/RawLoad.o: RawLoad.o
	$(ccd) -c RawLoad.cpp -o debug/RawLoad.o

Timer.o: Timer.cpp StdAfx.h Timer.h
	$(cc) -c Timer.cpp

debug/Timer.o: Timer.o
	$(ccd) -c Timer.cpp -o debug/Timer.o
