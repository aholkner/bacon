#include "Bacon.h"
#include "BaconInternal.h"

static Bacon_WindowResizeEventHandler s_ResizeHandler = nullptr;
static int s_Width = -1, s_Height = -1;

void Window_OnSizeChanged(int width, int height)
{
	s_Width = width;
	s_Height = height;
	if (s_ResizeHandler)
		s_ResizeHandler(width, height);
}

int Bacon_SetWindowResizeEventHandler(Bacon_WindowResizeEventHandler handler)
{
	s_ResizeHandler = handler;
	return Bacon_Error_None;
}

int Bacon_GetWindowSize(int* outWidth, int* outHeight)
{
	if (!outWidth || !outHeight)
		return Bacon_Error_InvalidArgument;
	
	*outWidth = s_Width;
	*outHeight = s_Height;
	
	return Bacon_Error_None;
}
