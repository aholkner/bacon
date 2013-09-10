#include "Bacon.h"
#include "BaconInternal.h"

static Bacon_TouchEventHandler s_Handler = nullptr;

namespace {
	const int MaxTouches = 11;
	
	struct Touch
	{
		float m_X, m_Y;
		int m_Pressed;
	};
	Touch s_Touches[MaxTouches];
}
void Touch_Init()
{
	s_Handler = nullptr;
	for (int i = 0; i < MaxTouches; ++i)
	{
		s_Touches[i].m_X = 0.f;
		s_Touches[i].m_Y = 0.f;
		s_Touches[i].m_Pressed = 0;
	}
}

void Touch_Shutdown()
{
}

static void UpdateDebugToggle()
{
	bool lowerLeft = false;
	bool upperRight = false;
	float radius = 32;
	int windowWidth, windowHeight;
	Bacon_GetWindowSize(&windowWidth, &windowHeight);
	
	for (int i = 0; i < 2; ++i)
	{
		if (!s_Touches[i].m_Pressed)
			continue;
		
		if (s_Touches[i].m_X < radius && s_Touches[i].m_Y > windowHeight - radius)
			lowerLeft = true;
		else if (s_Touches[i].m_X > windowWidth - radius && s_Touches[i].m_Y < radius)
			upperRight = true;
	}
	
	if (lowerLeft && upperRight)
		DebugOverlay_Toggle();
}

void Touch_SetTouchState(int touch, int state, float x, float y)
{
	if (touch < 0 || touch >= MaxTouches)
		return;

	bool oldPressed = s_Touches[touch].m_Pressed;
	s_Touches[touch].m_Pressed = state == Bacon_Touch_State_Pressed ? 1 : 0;
	s_Touches[touch].m_X = x;
	s_Touches[touch].m_Y = y;
	
	if (oldPressed != s_Touches[touch].m_Pressed && s_Handler)
		(s_Handler)(touch, state, x, y);
	
	if (!oldPressed && s_Touches[touch].m_Pressed)
		UpdateDebugToggle();
}

int Bacon_SetTouchEventHandler(Bacon_TouchEventHandler handler)
{
	s_Handler = handler;
	return Bacon_Error_None;
}

int Bacon_GetTouchState(int touch, int* outPressed, float* outX, float* outY)
{
	if (touch < 0 || touch >= MaxTouches)
		return Bacon_Error_InvalidArgument;
	
	*outPressed = s_Touches[touch].m_Pressed;
	*outX = s_Touches[touch].m_X;
	*outY = s_Touches[touch].m_Y;
	return Bacon_Error_None;
}
