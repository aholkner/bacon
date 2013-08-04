#include "Bacon.h"
#include "BaconInternal.h"

static unsigned int s_Buttons = 0;
static float s_MouseX = 0.f;
static float s_MouseY = 0.f;

static Bacon_MouseButtonEventHandler s_MouseButtonEventHandler = nullptr;
static Bacon_MouseScrollEventHandler s_MouseScrollEventHandler = nullptr;

void Mouse_Init()
{
}

void Mouse_Shutdown()
{
}

void Mouse_SetMousePosition(float x, float y)
{
	s_MouseX = x;
	s_MouseY = y;
}

void Mouse_SetMouseButtonPressed(int button, bool value)
{
	if (button >= 0 && button < 32)
	{
		unsigned int oldButtons = s_Buttons;
		if (value)
			s_Buttons |= (1 << button);
		else
			s_Buttons &= ~(1 << button);
		
		if (s_Buttons != oldButtons && s_MouseButtonEventHandler)
			s_MouseButtonEventHandler(button, value);
	}
}

void Mouse_OnMouseScrolled(float dx, float dy)
{
	if (s_MouseScrollEventHandler)
		s_MouseScrollEventHandler(dx, dy);
}

int Bacon_GetMousePosition(float* outX, float* outY)
{
	if (!outX || !outY)
		return Bacon_Error_InvalidArgument;
	
	*outX = s_MouseX;
	*outY = s_MouseY;
	
	return Bacon_Error_None;
}

int Bacon_SetMouseButtonEventHandler(Bacon_MouseButtonEventHandler handler)
{
	s_MouseButtonEventHandler = handler;
	return Bacon_Error_None;
}

int Bacon_SetMouseScrollEventHandler(Bacon_MouseScrollEventHandler handler)
{
	s_MouseScrollEventHandler = handler;
	return Bacon_Error_None;
}
