#include "Bacon.h"
#include "BaconInternal.h"

static Bacon_WindowResizeEventHandler s_ResizeHandler = nullptr;
static int s_Width = 640, s_Height = 480;
static float s_DeviceContentScale = 1.f;
static float s_ContentScale = 1.f;

void Window_Init()
{
    Bacon_SetWindowSize(s_Width, s_Height);
}

void Window_Shutdown()
{
}

void Window_OnSizeChanged(int width, int height)
{
	s_Width = width;
	s_Height = height;
	Bacon_Log(Bacon_LogLevel_Info, "Window size changed to %dx%d", width, height);
	if (s_ResizeHandler)
		s_ResizeHandler(width, height);
}

void Window_SetDeviceContentScale(float contentScale)
{
	Bacon_Log(Bacon_LogLevel_Info, "Device content scale = %f", contentScale);
	s_DeviceContentScale = contentScale;
	s_ContentScale = contentScale;
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

int Bacon_SetWindowContentScale(float contentScale)
{
	if (contentScale != 1.f && contentScale != s_DeviceContentScale)
	{
		Bacon_Log(Bacon_LogLevel_Error, "Cannot set content scale to %f, only 1.0 or the device content scale (currently %f)", contentScale, s_DeviceContentScale);
		return Bacon_Error_InvalidArgument;
	}
	
	if (Bacon_GetRunningState() != RunningState_None)
	{
		Bacon_Log(Bacon_LogLevel_Error, "Cannot set content scale while running");
		return Bacon_Error_Running;
	}
	
	Bacon_Log(Bacon_LogLevel_Info, "Window content scale set to %f", contentScale);
	s_ContentScale = contentScale;
	return Bacon_Error_None;
}

int Bacon_GetWindowContentScale(float* outContentScale)
{
	*outContentScale = s_ContentScale;
	return Bacon_Error_None;
}