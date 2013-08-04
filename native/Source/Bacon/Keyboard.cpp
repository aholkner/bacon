#include "Bacon.h"
#include "BaconInternal.h"

#include <cstring>
#include <deque>
using namespace std;

static char s_KeyStates[Key_MaxKey];
static Bacon_KeyEventHandler s_Handler = nullptr;

void Keyboard_Init()
{
	memset(s_KeyStates, 0, sizeof(s_KeyStates));
}

void Keyboard_Shutdown()
{
}

void Keyboard_SetKeyState(int key, bool value)
{
	if (key >= 0 && key < Key_MaxKey)
	{
		if (s_KeyStates[key] != (char)value)
		{
			s_KeyStates[key] = value;
			if (s_Handler)
				s_Handler(key, value);
		}
	}
}

int Bacon_GetKeyState(int key, int* outPressed)
{
	if (key < 0 || key >= Key_MaxKey)
		return Bacon_Error_InvalidArgument;

	*outPressed = s_KeyStates[key];
	return Bacon_Error_None;
}

int Bacon_SetKeyEventHandler(Bacon_KeyEventHandler handler)
{
	s_Handler = handler;
	return Bacon_Error_None;
}
