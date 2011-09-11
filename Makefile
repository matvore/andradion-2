cc = gcc -s -O2 -DNDEBUG -U_DEBUG
ccd = gcc -g -O2 -UNDEBUG -D_DEBUG
cc_gch = g++ -s -O2 -DNDEBUG -U_DEBUG -I.\DXInclude
ccd_gch = g++ -g -O2 -UNDEBUG -D_DEBUG -I.\DXInclude

cpp_core = Deeds.cpp Character.cpp Fire.cpp Fixed.cpp GammaEffects.cpp Glue.cpp LevelEnd.cpp Net.cpp Palette.cpp PowerUp.cpp Weather.cpp WinMain.cpp CompactMap.cpp Gfx.cpp GfxBasic.cpp GfxPretty.cpp Menu.cpp MusicLib.cpp Profiler.cpp RawLoad.cpp Timer.cpp Resource.cpp Keyboard.cpp Difficulty.cpp Sound.cpp

o_core = Deeds.o Character.o Fire.o Fixed.o GammaEffects.o Glue.o LevelEnd.o Net.o Palette.o PowerUp.o Weather.o WinMain.o Andradion2.o CompactMap.o Gfx.o GfxBasic.o GfxPretty.o Menu.o MusicLib.o Profiler.o RawLoad.o Timer.o Resource.o Keyboard.o Difficulty.o Sound.o

out = Andradion2.exe
debugout = and2debug.exe
cleaning = StdAfx.h.gch $(o_core) $(out) $(debugout) "Andradion 2.res"
rm = del /Q
gamebmps = "G:\Andradion 2\Resource\1charminn.bmp" "G:\Andradion 2\Resource\1charmins.bmp" "G:\Andradion 2\Resource\1charminw.bmp" "G:\Andradion 2\Resource\1cokens.bmp" "G:\Andradion 2\Resource\1cokew.bmp" "G:\Andradion 2\Resource\1evilturnern.bmp" "G:\Andradion 2\Resource\1evilturners.bmp" "G:\Andradion 2\Resource\1evilturnerw.bmp" "G:\Andradion 2\Resource\1koolaidguyn.bmp" "G:\Andradion 2\Resource\1koolaidguys.bmp" "G:\Andradion 2\Resource\1koolaidguyw.bmp" "G:\Andradion 2\Resource\1miltonn.bmp" "G:\Andradion 2\Resource\1miltons.bmp" "G:\Andradion 2\Resource\1miltonw.bmp" "G:\Andradion 2\Resource\1pepsin.bmp" "G:\Andradion 2\Resource\1pepsis.bmp" "G:\Andradion 2\Resource\1pepsiw.bmp" "G:\Andradion 2\Resource\1sallyn.bmp" "G:\Andradion 2\Resource\1sallys.bmp" "G:\Andradion 2\Resource\1sallyw.bmp" "G:\Andradion 2\Resource\1switzn.bmp" "G:\Andradion 2\Resource\1switzs.bmp" "G:\Andradion 2\Resource\1switzw.bmp" "G:\Andradion 2\Resource\1turnern.bmp" "G:\Andradion 2\Resource\1turners.bmp" "G:\Andradion 2\Resource\1turnerw.bmp" "G:\Andradion 2\Resource\2charminn.bmp" "G:\Andradion 2\Resource\2charmins.bmp" "G:\Andradion 2\Resource\2charminw.bmp" "G:\Andradion 2\Resource\2cokens.bmp" "G:\Andradion 2\Resource\2cokew.bmp" "G:\Andradion 2\Resource\2evilturnern.bmp" "G:\Andradion 2\Resource\2evilturners.bmp" "G:\Andradion 2\Resource\2evilturnerw.bmp" "G:\Andradion 2\Resource\2koolaidguyn.bmp" "G:\Andradion 2\Resource\2koolaidguys.bmp" "G:\Andradion 2\Resource\2koolaidguyw.bmp" "G:\Andradion 2\Resource\2miltonn.bmp" "G:\Andradion 2\Resource\2miltons.bmp" "G:\Andradion 2\Resource\2miltonw.bmp" "G:\Andradion 2\Resource\2pepsin.bmp" "G:\Andradion 2\Resource\2pepsis.bmp" "G:\Andradion 2\Resource\2pepsiw.bmp" "G:\Andradion 2\Resource\2sallyn.bmp" "G:\Andradion 2\Resource\2sallys.bmp" "G:\Andradion 2\Resource\2sallyw.bmp" "G:\Andradion 2\Resource\2switzn.bmp" "G:\Andradion 2\Resource\2switzs.bmp" "G:\Andradion 2\Resource\2switzw.bmp" "G:\Andradion 2\Resource\2turnern.bmp" "G:\Andradion 2\Resource\2turners.bmp" "G:\Andradion 2\Resource\2turnerw.bmp" "G:\Andradion 2\Resource\bazookan.bmp" "G:\Andradion 2\Resource\bazookas.bmp" "G:\Andradion 2\Resource\bazookaw.bmp" "G:\Andradion 2\Resource\bloodstain.bmp" "G:\Andradion 2\Resource\bullet.bmp" "G:\Andradion 2\Resource\decapitate1.bmp" "G:\Andradion 2\Resource\decapitate2.bmp" "G:\Andradion 2\Resource\explosion.bmp" "G:\Andradion 2\Resource\Health.bmp" "G:\Andradion 2\Resource\mgn.bmp" "G:\Andradion 2\Resource\mgs.bmp" "G:\Andradion 2\Resource\mgw.bmp" "G:\Andradion 2\Resource\pistoln.bmp" "G:\Andradion 2\Resource\pistols.bmp" "G:\Andradion 2\Resource\pistolw.bmp" "G:\Andradion 2\palettes\extracolors.bmp"

all: $(debugout)

release: $(out)

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

Andradion2.o: Andradion\ 2.rc Glue.h
	rc /i "C:\Program Files\MS Platform SDK\Include" /r "Andradion 2.rc"
	windres -iAndradion\ 2.res -oAndradion2.o
	$(rm) "Andradion 2.res"

$(out): $(cpp_core) Andradion2.o
	$(rm) StdAfx.h.gch
	$(cc_gch) StdAfx.h -o StdAfx.h.gch
	$(cc) -o $(out) $(cpp_core) Andradion2.o -lstdc++ -lwinmm -ldxguid -lgdi32 -lole32 -mwindows -s
	$(rm) StdAfx.h.gch

$(debugout): $(o_core)
	$(ccd) -o $(debugout) $(o_core) -lstdc++ -lwinmm -ldxguid -lgdi32 -lole32

StdAfx.h.gch: StdAfx.h
	$(ccd_gch) -c StdAfx.h -o StdAfx.h.gch

Deeds.o: Deeds.cpp StdAfx.h.gch Fixed.h Deeds.h
	$(ccd) -c Deeds.cpp

Character.o: Character.cpp StdAfx.h.gch Fixed.h GammaEffects.h Net.h PowerUp.h Character.h Fire.h Glue.h Logger.h Gfx.h GfxBasic.h GfxPretty.h Keyboard.h Difficulty.h Sound.h
	$(ccd) -c Character.cpp

Fire.o: Fire.cpp StdAfx.h.gch Fixed.h Net.h Gfx.h Character.h Fire.h Glue.h Keyboard.h Sound.h
	$(ccd) -c Fire.cpp

Fixed.o: Fixed.cpp Fixed.h StdAfx.h.gch
	$(ccd) -c Fixed.cpp

GammaEffects.o: GammaEffects.cpp StdAfx.h.gch Gfx.h Fixed.h GammaEffects.h Logger.h Palette.h 
	$(ccd) -c GammaEffects.cpp

Glue.o: Glue.cpp StdAfx.h.gch Gfx.h CompactMap.h Fixed.h Weather.h Fire.h Character.h Glue.h Gfx.h GfxBasic.h GfxPretty.h Deeds.h LevelEnd.h PowerUp.h Net.h GammaEffects.h Logger.h Profiler.h MusicLib.h Timer.h Menu.h RawLoad.h Buffer.h Resource.h Keyboard.h Difficulty.h Sound.h
	$(ccd) -c Glue.cpp


LevelEnd.o: LevelEnd.cpp StdAfx.h.gch Fixed.h LevelEnd.h
	$(ccd) -c LevelEnd.cpp

Net.o: Net.cpp StdAfx.h.gch Net.h Logger.h
	$(ccd) -c Net.cpp

Palette.o: Palette.cpp Fixed.h StdAfx.h.gch Gfx.h Logger.h Palette.h
	$(ccd) -c Palette.cpp

PowerUp.o: PowerUp.cpp StdAfx.h.gch Fixed.h Gfx.h Character.h Glue.h PowerUp.h Net.h MusicLib.h Sound.h
	$(ccd) -c PowerUp.cpp

Weather.o: Weather.cpp StdAfx.h.gch Fixed.h Weather.h Logger.h Gfx.h Character.h Glue.h Sound.h
	$(ccd) -c Weather.cpp

WinMain.o: WinMain.cpp StdAfx.h.gch Fixed.h Logger.h Character.h Glue.h
	$(ccd) -c WinMain.cpp

CompactMap.o: CompactMap.cpp StdAfx.h.gch CompactMap.h Gfx.h Logger.h Buffer.h Resource.h
	$(ccd) -c CompactMap.cpp

Gfx.o: Gfx.cpp StdAfx.h.gch Gfx.h Buffer.h Logger.h
	$(ccd) -c Gfx.cpp

GfxBasic.o: GfxBasic.cpp StdAfx.h.gch Gfx.h GfxBasic.h Logger.h
	$(ccd) -c GfxBasic.cpp

GfxPretty.o: GfxPretty.cpp StdAfx.h.gch Gfx.h GfxPretty.h Logger.h
	$(ccd) -c GfxPretty.cpp

Menu.o: Menu.cpp StdAfx.h.gch Gfx.h Menu.h Logger.h Fixed.h
	$(ccd) -c Menu.cpp

MusicLib.o: MusicLib.cpp StdAfx.h.gch MusicLib.h Logger.h Resource.h
	$(ccd) -c MusicLib.cpp

Profiler.o: Profiler.cpp StdAfx.h.gch Profiler.h
	$(ccd) -c Profiler.cpp

RawLoad.o: RawLoad.cpp StdAfx.h.gch Logger.h
	$(ccd) -c RawLoad.cpp

Timer.o: Timer.cpp StdAfx.h.gch Timer.h
	$(ccd) -c Timer.cpp

Resource.o: Resource.cpp Resource.h StdAfx.h.gch
	$(ccd) -c Resource.cpp

Keyboard.o: Keyboard.cpp StdAfx.h.gch Keyboard.h
	$(ccd) -c Keyboard.cpp

Difficulty.o: Difficulty.cpp StdAfx.h.gch Difficulty.h Fixed.h
	$(ccd) -c Difficulty.cpp

Sound.o: Sound.cpp StdAfx.h.gch Sound.h RawLoad.h Logger.h Resource.h Fixed.h
	$(ccd) -c Sound.cpp
