#include "StdAfx.h"
#include "Controller.h"
#include "Array.h"

using namespace std;

static int global_action_buttons[GLOBAL_ACTIONS];
static int controller_count = 0;

static set<int> active_joysticks;
static Array<JOYINFOEX> joystick_states;
