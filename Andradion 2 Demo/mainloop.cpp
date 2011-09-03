#include "Andradion 2.h"

GraphLib::CGraphics *gr;

bool mainloop(bool stop) {
	using namespace GraphLib;

	static bool intro_ran = false;

	if(intro_ran)
	{
		return gameloop(stop);
	}
	
	intro_ran = intrloop(stop);
	return false;
}
